# This is the Xcode app icon and the files used to create it.

When built with Xcode 16 or below, the original free-form icon is generated from the asset catalog, and is used on macOS 15 and below.  On macOS 26 and above, the system automatically reduces it and puts it inside a grey, rounded square (squircle) background.

When built with Xcode 26 or above, the "new style" (Liquid Glass) icon is built into the app and a flattened version is generated for pre-macOS 26 systems.  Both are rendered inside a rounded square background, but on macOS 26+ the Liquid Glass highlighting and gradients are applied.

All of the work was done in Apple's Pixelmator Pro and Icon Composer apps.

```
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
| Imported the above shape as a layer into the Photoshop icon template, provided by Apple, for sizing,
| alignment, and general tweaking.  I think of this as the "master" file.
|
|
PWSafeLogo 2025-07-21.svg  -------------------------|
| Re-exported the logo layer as a SVG for import    |
| into Apple's Icon Composer app to apply the       |
| Liquid Glass gradients and highlights.            |
|                                                   |
|                                                   |
AppIcon.icon                                        PWSafeLogo256x256.png
  The final Liquid Glass App Icon, used directly    | Re-saved the vector shape as a png.  This is the
  by Xcode 26, edited with Icon Composer.           | icon used by Xcode 16 and older.  A copy of this
                                                    | is in the Xcode asset catalog.
                                                    |
                                                    |
                                                    AppIcon.icns
                                                      This file is used by create-dmg as the volume icon.
                                                      This is what Xcode 16.4 made of the above file and
                                                      included in the app bundle. With Xcode 26, the
                                                      generated .icns file contains a flattened version
                                                      of the Liquid Glass Icon.
```