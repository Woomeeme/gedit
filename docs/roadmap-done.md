gedit roadmap - done tasks
==========================

Tepl-ification of the gedit core
--------------------------------

- gedit 3.36:
	- Start to use the Tepl library.
	- Use some Tepl utility functions.
	- Use TeplFileMetadata, remove GeditMetadataManager.
- gedit 3.38:
	- Move some utility functions to the Tepl library.
	- Refactor and move some I/O error infobars to Tepl.
	- GeditView now inherits from TeplView.
	- Port to the new Tepl metadata API.
	- Use TeplStyleSchemeChooserWidget in the preferences dialog.
	- Create GeditFactory class, subclass of TeplAbstractFactory.
- gedit 40:
	- Use `tepl_pango_font_description_to_css()`.
	- Use TeplLanguageChooser's, for choosing a language for the syntax
	  highlighting. Remove GeditHighlightModeSelector and
	  GeditHighlightModeDialog.
	- Use TeplProgressInfoBar. Remove GeditProgressInfoBar.
	- GeditDocument now inherits from TeplBuffer, start to use the
	  TeplBuffer and TeplFile APIs.

Links:
- https://wiki.gnome.org/Projects/Tepl

Tepl-ification of the gedit plugins
-----------------------------------

- gedit 40:
	- Draw Spaces plugin: new implementation based on TeplSpaceDrawerPrefs.

Other done tasks in gedit plugins
---------------------------------

- gedit 40:
	- Smart Spaces plugin: new implementation based on a GtkSourceView
	  feature.

New version of gedit on Windows
-------------------------------

[gedit is now available on the Microsoft Store](https://www.microsoft.com/store/apps/9PL1J21XF0PT).
It was done during the GNOME 3.38 development cycle. The integration with
Windows is not perfect, but it works. It is planned to improve gedit for
Windows over time.

Documentation for contributors
------------------------------

Write a guide to get started with gedit development.

Done during the GNOME 3.34 development cycle.
