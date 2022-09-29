Common gedit bugs
=================

This page documents common bugs in gedit. If you find your problem in this page,
_please do not report a new bug for it_.

Problem with very long lines
----------------------------

Very long lines (e.g. a wrapped line that takes the whole screen) are not well
supported by gedit, there can be performance problems or freezes.

[GitLab issue](https://gitlab.gnome.org/GNOME/gtk/issues/229).

Problem with very large files
-----------------------------

Large files are not well supported, gedit should ask for confirmation when
opening such files, providing a solution.

[GitLab issue](https://gitlab.gnome.org/GNOME/gedit/issues/197).

Hyphen/dash inserted for text wrapping
--------------------------------------

It should be disabled in a text editor like gedit, the hyphen is not part of
the content.

[GitLab issue](https://gitlab.gnome.org/GNOME/gedit/issues/365).

View does not scroll to the end of the text in some cases (text cut off)
------------------------------------------------------------------------

[GitLab issue](https://gitlab.gnome.org/GNOME/gedit/issues/42).

See [this comment](https://gitlab.gnome.org/GNOME/gedit/-/issues/42#note_774722)
and the following, disabling GTK animations fixes the bug.
