These are the app icons used by Xcode to build the app and the files that were 
used to create them.  When built with Xcode 26, both the "old" and "new" (Liquid Glass)
styles are built into the app and the selection is made at run time, i.e. macOS 26
uses the new Liquid Glass style icons (inside their rounded square) while macOS 15 and
earlier continue to use the original free-form style icon.  

To make that work, you need to use an undocumented Xcode 26 build option.  Without this option, 
the old icon is ignored and a version of the new icon, modified to match older systems, is used.
ASSETCATALOG_OTHER_FLAGS = --enable-icon-stack-fallback-generation=disabled

Building in Xcode 16 or earlier, that option causes an error.  Removing it will produce a
build that only uses the old style icon on all systems.  On macOS 26, old style icons are reduced 
in size and placed onto a grey background to make them match the new size and shape.


All of the work was done in Pixelmator Pro and Apple's new Icon Composer app.

cpane.png
| Original image, lifted from the pwsafe.org website.
|
|
cpane logo.svg
| Just the red pixels converted to a vector shape for better scaling.
|
|
PWSafeLogo.pxd
| Pixelmator Pro working file.
| Imported the above shape as a layer into the Photoshop icon template provided by Apple for sizing,
| alignment, and general tweaking.  I think of this as the "master" file.
|
|
PWSafeLogo 2025-07-21.svg  -------------------------|
| Re-exported the logo layer as an SVG for import   |
| into Icon Composer to apply the                   |
| Liquid Glass gradients and highlights.            |
|                                                   |
|                                                   |
AppIcon.icon                                        PWSafeLogo256x256.png
  "Final" Liquid Glass App Icon, used directly      | Re-saved the vector shape as a png with transparency.
  by Xcode, edited with Icon Composer.              | This is the icon used on macOS 15 and older.
                                                    | A copy of this is in the Xcode asset catalog.
                                                    |
                                                    |
                                                    AppIcon.icns
                                                      This is what Xcode 16.4 made of the above file and
                                                      included in the app bundle.
                                                      Xcode 26 no longer generates this file during the build
                                                      when using both old and new styles, but it's still used by
                                                      create-dmg as the volume icon.
