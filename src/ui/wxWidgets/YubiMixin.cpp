/*
 * Copyright (c) 2003-2025 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

/** \file YubiMixin.cpp
* 
*/

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "os/unix/PWYubi.h"
#include "os/sleep.h"

#include "YubiMixin.h"

#include <iomanip>
#include <sstream>

int YubiMixin::s_pollingInterval = YubiMixin::POLLING_INTERVAL_DEFAULT;

void YubiMixin::SetupMixin(wxEvtHandler *eventHandler, wxWindow *btn, wxWindow *status, int timerId)
{
  m_prompt1 = _("Activate YubiKey"); // change via SetPrompt1
  m_prompt2 = _("Touch YubiKey's button"); // change via SetPrompt2
  m_btn = btn;
  m_status = status;
  m_present = !IsYubiInserted(); // lie to trigger correct actions in timer even
  if ((m_btn != nullptr) && (m_status != nullptr)) {
    // Hide Yubi controls if user doesn't have one:
    m_btn->Show(yubiExists());
    m_status->Show(yubiExists());
    if (IsPollingEnabled() && (timerId != YubiMixin::POLLING_TIMER_NONE)) {
      m_pollingTimer = new wxTimer(eventHandler, timerId);
      m_pollingTimer->Start(GetPollingInterval());
    }
  }
}

bool YubiMixin::yubiExists() const
{
  return PWYubi::YubiExists();
}

void YubiMixin::yubiInserted(void)
{
  m_btn->Enable(true);
  m_status->SetForegroundColour(wxNullColour);
  m_status->SetLabel(m_prompt1);
}

void YubiMixin::yubiRemoved(void)
{
  m_btn->Enable(false);
  m_status->SetForegroundColour(wxNullColour);
  m_status->SetLabel(_("Insert YubiKey"));
}

bool YubiMixin::IsYubiInserted() const
{
  const PWYubi yubi;
  return yubi.IsYubiInserted();
}

void YubiMixin::HandlePollingTimer()
{
  // Show Yubi controls when inserted first time:
  if (yubiExists() && (!m_btn->IsShown() || !m_status->IsShown())) {
    m_btn->Show(true);
    m_status->Show(true);
    updateLayout();
  }

  // Currently hmac check is blocking (ugh), so no need to check here
  // if a request is in-progress.
  bool inserted = IsYubiInserted();
  if (inserted != m_present) {
    m_present = inserted;
    UpdateStatus();
  }
}

void YubiMixin::SetPollingInterval(int value) {
  if (value <= YubiMixin::POLLING_INTERVAL_OFF) {
    s_pollingInterval = YubiMixin::POLLING_INTERVAL_OFF; // indicates to disable the polling
  }
  else if (value < YubiMixin::POLLING_INTERVAL_MIN /* ms */) {
    s_pollingInterval = YubiMixin::POLLING_INTERVAL_MIN; // limit the polling interval to a minimum
  }
  else if (value > YubiMixin::POLLING_INTERVAL_MAX /* ms */) {
    s_pollingInterval = YubiMixin::POLLING_INTERVAL_MAX; // limit the polling interval to a maximum
  }
  else {
    s_pollingInterval = value;  // the user defined polling interval
  }
}

void YubiMixin::UpdateStatus()
{
  if (m_present)
    yubiInserted();
  else
    yubiRemoved();

  updateLayout();
}

void YubiMixin::updateLayout()
{
  // Update the dialog's layout as additional controls
  // have appeared or the prompt has changed.
  if (m_btn) {
    auto *parent = m_btn->GetParent();
    if (parent != nullptr) {
      parent->GetSizer()->Layout();
    }
  }
}

bool YubiMixin::PerformChallengeResponse(wxWindow *win,
            const StringX &challenge,
            StringX &response,
            bool oldYubiChallenge)
{
  bool retval = false;
  m_status->SetForegroundColour(wxNullColour);
  m_status->SetLabel(m_prompt2);
  ::wxSafeYield(win); // get text to update
  BYTE chalBuf[PWYubi::SHA1_MAX_BLOCK_SIZE];
  memset(chalBuf, 0, PWYubi::SHA1_MAX_BLOCK_SIZE);
  BYTE chalLength;

  /*
   * Following mess is to fix BR1201:
   * Root cause: sizeof(TCHAR) on Windows is 2, but 4 on Linux
   * This caused a challenge of L"abc" to be {a\0b\0c\0} on Windows,
   * but {a\0\0\0b\0\0\0c\0\0\0} on Linux.
   *
   * Starting with this fix (0.94, 16 July 2014), Linux
   * will generate the same challenge as Windows UNLESS
   * oldYubiChallenge is true.
   * This will allow users to read pre-0.94 databases
   * but save them in a manner that will be compatible
   * with Windows.
   */
  if (oldYubiChallenge) {
    chalLength = BYTE(challenge.length() * sizeof(TCHAR));
    if (chalLength > PWYubi::SHA1_MAX_BLOCK_SIZE)
      chalLength = PWYubi::SHA1_MAX_BLOCK_SIZE;
  
    memcpy(chalBuf, challenge.c_str(), chalLength);
  } else { // emulate Windows challenge
    chalLength = BYTE(challenge.length() * 2);
    if (chalLength > PWYubi::SHA1_MAX_BLOCK_SIZE)
      chalLength = PWYubi::SHA1_MAX_BLOCK_SIZE;
    const BYTE *b = reinterpret_cast<const BYTE *>(challenge.c_str());
    for (BYTE i = 0; i < chalLength; i+=2, b+=sizeof(TCHAR)) {
      chalBuf[i] = *b; chalBuf[i+1] = *(b+1);
    }
  }

  PWYubi yubi;
  if (yubi.RequestHMacSHA1(chalBuf, chalLength)) {
    unsigned char hmac[PWYubi::RESPLEN];
    PWYubi::RequestStatus status = PWYubi::PENDING;
    do {
      status = yubi.GetResponse(hmac);
      if (status == PWYubi::PENDING)
        pws_os::sleep_ms(250); // Ugh.
      ::wxSafeYield(win); // so as not to totally freeze the app...
    } while (status == PWYubi::PENDING);
    if (status == PWYubi::DONE) {
#if 0
      for (unsigned i = 0; i < sizeof(hmac); i++)
        std::cerr << std::hex << std::setw(2) << (int)hmac[i];
      std::cerr << std::endl;
#endif
      // The returned hash is the passkey
      response = Bin2Hex(hmac, PWYubi::RESPLEN);
      retval = true;
    } else {
      if (status == PWYubi::TIMEOUT) {
        m_status->SetForegroundColour(*wxRED);
        m_status->SetLabel(_("Timeout - please try again"));
      } else { // error
        m_status->SetForegroundColour(*wxRED);
        m_status->SetLabel(_("Error: Bad response from YubiKey"));
      }
    }
  } else {
    m_status->SetForegroundColour(*wxRED);
    m_status->SetLabel(_("Error: Unconfigured YubiKey?"));
  }
  return retval;
}

StringX YubiMixin::Bin2Hex(const unsigned char *buf, int len) const
{
  std::wostringstream os;
  os << std::setw(2);
  os << std::setfill(L'0');
  for (int i = 0; i < len; i++) {
    os << std::hex << std::setw(2) << int(buf[i]);
  }
  return StringX(os.str().c_str());
}
