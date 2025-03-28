'
' Copyright (c) 2003-2025 Rony Shapiro <ronys@pwsafe.org>.
' All rights reserved. Use of the code is allowed under the
' Artistic License 2.0 terms, as specified in the LICENSE file
' distributed with this code, or available from
' http://www.opensource.org/licenses/artistic-license-2.0.php
'

' This section creates C++ source and header files from core.rc2
' for use by non-MFC configurations

' For the stdout.WriteLine to work, this Post-Build Event script
' MUST be executed via cscript command.

Option Explicit

If Instr(1, WScript.FullName, "cscript.exe", vbTextCompare) = 0 then
  MsgBox " Host: " & WScript.FullName & vbCRLF & _
         "This script must be executed by cscript.exe", _
         vbCritical, _
         "Error: " & Wscript.ScriptFullName
  ' return error code to caller
  Wscript.Quit(99)
End If

Dim stdout, stdFSO
Set stdFSO = CreateObject("Scripting.FileSystemObject")
Set stdout = stdFSO.GetStandardStream(1)

Dim objShell, objFSO, rc
Set objShell = WScript.CreateObject("WScript.Shell")
Set objFSO = CreateObject("Scripting.FileSystemObject")

Dim strProjectDir, strConfig, strConfigLC, bMFC
Dim strCPPFile, strHFile, bCPPExists, bHExists
strProjectDir = objShell.ExpandEnvironmentStrings("%ProjectDir%")
strConfig = objShell.ExpandEnvironmentStrings("%ConfigurationName%")

strConfigLC = LCase(strConfig)

' Rather than assume that wxWidgets or other non-MFC configurations end
' with 'wx', just use known MFC configurations
Select Case strConfigLC
  Case "debug", "release", "debug64", "release64"
    bMFC = true
  Case "debugx", "debugx64"
    bMFC = true
  Case "demo", "releasex", "demo64", "releasex64"
    bMFC = true
  Case Else
    bMFC = false
End Select

' Remove double quotes
strProjectDir = Replace(strProjectDir, Chr(34), "", 1, -1, vbTextCompare)

' Ensure ends with a back slash
If Right(strProjectDir, 1) <> "\" Then
  strProjectDir = strProjectDir & "\"
End If

strCPPFile = strProjectDir + "core_st.cpp"
strHFile = strProjectDir + "core_st.h"

' Check if any already exist
bCPPExists = objFSO.FileExists(strCPPFile)
bHExists = objFSO.FileExists(strHFile)

rc = 0
stdout.WriteLine " "

If bMFC Then
' for MFC, fake script rc2cpp.pl by creating empty files
  If (Not bCPPExists Or Not bHExists) Then
    ' Recreate dummy versions of both files
    Call WriteMFCDummyHFile(strHFile)
    Call WriteMFCDummyCPPFile(strCPPFile)

    stdout.WriteLine " Dummy versions of core_st.cpp & core_st.h have been created"
  End If
  stdout.WriteLine " "
  WScript.Quit(rc)
End If

Dim cmd
' Create command string
cmd = "C:\local\cygwin64\bin\perl.exe -w " & "..\..\Misc\rc2cpp.pl" & " " & "core.rc2"

' If prefix script by "Perl", may get return code:
'    80070002 - System cannot find file (i.e. "Perl") if not in the path, or
' If just execute the script, may get return code:
'    80070483 - No program associated with the '.pl' extension
Dim objWshScriptExec, objStdOut, strLine

stdout.WriteLine " Executing script: " & cmd

On Error Resume Next
Set objWshScriptExec = objShell.Exec(cmd)
If Err.Number <> 0 Then
  rc = Err.Number
End If
Err.Clear
On Error Goto 0

If rc = 0 Then
  Set objStdOut = objWshScriptExec.StdOut

  Do While objWshScriptExec.Status = 0
     WScript.Sleep 100
  Loop

  While Not objStdOut.AtEndOfStream
    strLine = objStdOut.ReadLine
    stdout.WriteLine "  " & strLine
  Wend

  stdout.WriteLine " Script ended with return code: " & objWshScriptExec.ExitCode
Else
  If Hex(rc) = "80070483" Or Hex(rc) = "80070002" Then
    stdout.WriteLine " *** Can't find Perl on your computer to run conversion script rc2cpp.pl" & vbCRLF & _
           " *** Please install it or create core_st.cpp & core_st.h from core.rc2 manually"
    If bCPPExists Or bHExists Then
      stdout.WriteLine " *** Any existing copies of core_st.cpp & core_st.h may be out of date."
    End If
  End If
  rc = 99
End If

stdout.WriteLine " "

' Tidy up
Set objWshScriptExec = Nothing
Set objStdOut = Nothing
Set objShell = Nothing
Set objFSO = Nothing
Set stdout = Nothing
Set stdFSO = Nothing

WScript.Quit(rc)

' -----------------------------

Sub WriteMFCDummyHFile(strHFile)
  Dim objFSO, objFile
  Set objFSO = CreateObject("Scripting.FileSystemObject")

  If Not objFSO.FileExists(strHFile) Then
    Set objFile = objFSO.CreateTextFile(strHFile)
  End If

  Set objFile = nothing

  ' OpenTextFile Method needs a Const value
  ' ForAppending = 8 ForReading = 1, ForWriting = 2
  Const ForWriting = 2

  Set objFile = objFSO.OpenTextFile(strHFile, ForWriting, True)
  objFile.WriteLine("#ifndef __core_st_H")
  objFile.WriteLine("#define __core_st_H")
  objFile.WriteLine("/**")
  objFile.WriteLine(" * Declaration of string table map")
  objFile.WriteLine(" *")
  objFile.WriteLine(" * THIS FILE IS AUTOMATICALLY GENERATED. DO NOT EDIT.")
  objFile.WriteLine(" *")
  objFile.WriteLine(" * This is a dummy file for MFC builds")
  objFile.WriteLine(" *")
  objFile.WriteLine(" */")
  objFile.WriteLine("")
  objFile.WriteLine("#ifdef __WX__")
  objFile.WriteLine("#error " & Chr(34) & "You must regenerate core_st.h & core_st.cpp" & Chr(34))
  objFile.WriteLine("#endif")
  objFile.WriteLine("")
  objFile.WriteLine("#endif /* __core_st_H */")

  objFile.Close
  Set objFile = Nothing
  Set objFSO = Nothing
End Sub

Sub WriteMFCDummyCPPFile(strCPPFile)
  Dim objFSO, objFile
  Set objFSO = CreateObject("Scripting.FileSystemObject")

  If Not objFSO.FileExists(strHFile) Then
    Set objFile = objFSO.CreateTextFile(strCPPFile)
  End If

  Set objFile = Nothing

  ' OpenTextFile Method needs a Const value
  ' ForAppending = 8 ForReading = 1, ForWriting = 2
  Const ForWriting = 2

  Set objFile = objFSO.OpenTextFile(strCPPFile, ForWriting, True)
  objFile.WriteLine("/**")
  objFile.WriteLine(" * Definition of string table map")
  objFile.WriteLine(" *")
  objFile.WriteLine(" * THIS FILE IS AUTOMATICALLY GENERATED. DO NOT EDIT.")
  objFile.WriteLine(" *")
  objFile.WriteLine(" * This is a dummy file for MFC builds")
  objFile.WriteLine(" *")
  objFile.WriteLine(" */")
  objFile.WriteLine("")
  objFile.WriteLine("#ifdef __WX__")
  objFile.WriteLine("#include " & Chr(34) & ".\core_st.h" & Chr(34))
  objFile.WriteLine("#endif")

  objFile.Close
  Set objFile = Nothing
  Set objFSO = Nothing
End Sub
