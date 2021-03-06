/* Editor.cpp
 *
 * Copyright (C) 1992-2011 Paul Boersma
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
/* some GTK stuff added by Stefan de Konink in 2008 and Franz Brausse in 2010 */

#include <time.h>
#include "ScriptEditor.h"
#include "machine.h"
#include "EditorM.h"
#include "praat_script.h"
#include "Preferences.h"

#include "enums_getText.h"
#include "Editor_enums.h"
#include "enums_getValue.h"
#include "Editor_enums.h"

Thing_implement (Editor, Thing, 0);

/********** PREFERENCES **********/

static struct {
	struct {
		bool eraseFirst;
		enum kEditor_writeNameAtTop writeNameAtTop;
	} picture;
} preferences;

void Editor_prefs (void) {
	Preferences_addBool (L"Editor.picture.eraseFirst", & preferences.picture.eraseFirst, true);
	Preferences_addEnum (L"Editor.picture.writeNameAtTop", & preferences.picture.writeNameAtTop, kEditor_writeNameAtTop, DEFAULT);
}

/********** class EditorCommand **********/

Thing_implement (EditorCommand, Thing, 0);

void structEditorCommand :: v_destroy () {
	Melder_free (itemTitle);
	Melder_free (script);
	forget (d_uiform);
	EditorCommand_Parent :: v_destroy ();
}

/********** class EditorMenu **********/

Thing_implement (EditorMenu, Thing, 0);

void structEditorMenu :: v_destroy () {
	Melder_free (menuTitle);
	forget (commands);
	EditorMenu_Parent :: v_destroy ();
}

/********** functions **********/

static void commonCallback (GUI_ARGS) {
	GUI_IAM (EditorCommand);
	#if gtk
		if (G_OBJECT_TYPE (w) == GTK_TYPE_RADIO_MENU_ITEM && ! gtk_check_menu_item_get_active (GTK_CHECK_MENU_ITEM (w))) {
			return;
		}
	#endif
	if (my d_editor && my d_editor -> v_scriptable () && ! wcsstr (my itemTitle, L"...")) {
		UiHistory_write (L"\n");
		UiHistory_write (my itemTitle);
	}
	try {
		my commandCallback (my d_editor, me, NULL, NULL, NULL);
	} catch (MelderError) {
		Melder_error_ ("Menu command \"", my itemTitle, "\" not completed.");
		Melder_flushError (NULL);
	}
}

GuiObject EditorMenu_addCommand (EditorMenu me, const wchar *itemTitle, long flags,
	void (*commandCallback) (Editor me, EditorCommand cmd, UiForm sendingForm, const wchar *sendingString, Interpreter interpreter))
{
	autoEditorCommand thee = Thing_new (EditorCommand);
	thy d_editor = my d_editor;
	thy menu = me;
	thy itemTitle = Melder_wcsdup (itemTitle);
	thy itemWidget =
		commandCallback == NULL ? GuiMenu_addSeparator (my menuWidget) :
		flags & Editor_HIDDEN ? NULL :
		GuiMenu_addItem (my menuWidget, itemTitle, flags, commonCallback, thee.peek());   // DANGLE BUG: me can be killed by Collection_addItem(), but EditorCommand::destroy doesn't remove the item
	thy commandCallback = commandCallback;
	GuiObject result = thy itemWidget;
	Collection_addItem (my commands, thee.transfer());
	return result;
}

/*GuiObject EditorCommand_getItemWidget (EditorCommand me) { return my itemWidget; }*/

EditorMenu Editor_addMenu (Editor me, const wchar *menuTitle, long flags) {
	autoEditorMenu thee = Thing_new (EditorMenu);
	thy d_editor = me;
	thy menuTitle = Melder_wcsdup (menuTitle);
	thy menuWidget = GuiMenuBar_addMenu (my menuBar, menuTitle, flags);
	thy commands = Ordered_create ();
	EditorMenu result = thee.peek();
	Collection_addItem (my menus, thee.transfer());
	return result;
}

/*GuiObject EditorMenu_getMenuWidget (EditorMenu me) { return my menuWidget; }*/

GuiObject Editor_addCommand (Editor me, const wchar *menuTitle, const wchar *itemTitle, long flags,
	void (*commandCallback) (Editor me, EditorCommand cmd, UiForm sendingForm, const wchar *sendingString, Interpreter interpreter))
{
	try {
		long numberOfMenus = my menus -> size;
		for (long imenu = 1; imenu <= numberOfMenus; imenu ++) {
			EditorMenu menu = (EditorMenu) my menus -> item [imenu];
			if (wcsequ (menuTitle, menu -> menuTitle))
				return EditorMenu_addCommand (menu, itemTitle, flags, commandCallback);
		}
		Melder_throw ("Menu \"", menuTitle, L"\" does not exist.");
	} catch (MelderError) {
		Melder_throw ("Command \"", itemTitle, "\" not inserted in menu \"", menuTitle, ".");
	}
}

static void Editor_scriptCallback (Editor me, EditorCommand cmd, UiForm sendingForm, const wchar *sendingString, Interpreter interpreter) {
	(void) sendingForm;
	(void) sendingString;
	(void) interpreter;
	DO_RunTheScriptFromAnyAddedEditorCommand (me, cmd -> script);
}

GuiObject Editor_addCommandScript (Editor me, const wchar *menuTitle, const wchar *itemTitle, long flags,
	const wchar *script)
{
	try {
		long numberOfMenus = my menus -> size;
		for (long imenu = 1; imenu <= numberOfMenus; imenu ++) {
			EditorMenu menu = (EditorMenu) my menus -> item [imenu];
			if (wcsequ (menuTitle, menu -> menuTitle)) {
				autoEditorCommand cmd = Thing_new (EditorCommand);
				cmd -> d_editor = me;
				cmd -> menu = menu;
				cmd -> itemTitle = Melder_wcsdup_f (itemTitle);
				cmd -> itemWidget = script == NULL ? GuiMenu_addSeparator (menu -> menuWidget) :
					GuiMenu_addItem (menu -> menuWidget, itemTitle, flags, commonCallback, cmd.peek());   // DANGLE BUG
				cmd -> commandCallback = Editor_scriptCallback;
				if (wcslen (script) == 0) {
					cmd -> script = Melder_wcsdup_f (L"");
				} else {
					structMelderFile file = { 0 };
					Melder_relativePathToFile (script, & file);
					cmd -> script = Melder_wcsdup_f (Melder_fileToPath (& file));
				}
				GuiObject result = cmd -> itemWidget;
				Collection_addItem (menu -> commands, cmd.transfer());
				return result;
			}
		}
		Melder_throw ("Menu \"", menuTitle, L"\" does not exist.");
	} catch (MelderError) {
		Melder_throw ("Command \"", itemTitle, "\" not inserted in menu \"", menuTitle, ".");
	}
}

void Editor_setMenuSensitive (Editor me, const wchar *menuTitle, int sensitive) {
	int numberOfMenus = my menus -> size;
	for (int imenu = 1; imenu <= numberOfMenus; imenu ++) {
		EditorMenu menu = (EditorMenu) my menus -> item [imenu];
		if (wcsequ (menuTitle, menu -> menuTitle)) {
			GuiObject_setSensitive (menu -> menuWidget, sensitive);
			return;
		}
	}
}

EditorCommand Editor_getMenuCommand (Editor me, const wchar *menuTitle, const wchar *itemTitle) {
	int numberOfMenus = my menus -> size;
	for (int imenu = 1; imenu <= numberOfMenus; imenu ++) {
		EditorMenu menu = (EditorMenu) my menus -> item [imenu];
		if (wcsequ (menuTitle, menu -> menuTitle)) {
			int numberOfCommands = menu -> commands -> size, icommand;
			for (icommand = 1; icommand <= numberOfCommands; icommand ++) {
				EditorCommand command = (EditorCommand) menu -> commands -> item [icommand];
				if (wcsequ (itemTitle, command -> itemTitle))
					return command;
			}
		}
	}
	Melder_throw ("Command \"", itemTitle, "\" not found in menu \"", menuTitle, "\".");
}

void Editor_doMenuCommand (Editor me, const wchar *commandTitle, const wchar *arguments, Interpreter interpreter) {
	int numberOfMenus = my menus -> size;
	for (int imenu = 1; imenu <= numberOfMenus; imenu ++) {
		EditorMenu menu = (EditorMenu) my menus -> item [imenu];
		long numberOfCommands = menu -> commands -> size;
		for (long icommand = 1; icommand <= numberOfCommands; icommand ++) {
			EditorCommand command = (EditorCommand) menu -> commands -> item [icommand];
			if (wcsequ (commandTitle, command -> itemTitle)) {
				command -> commandCallback (me, command, NULL, arguments, interpreter);
				return;
			}
		}
	}
	Melder_throw ("Command not available in ", my classInfo -> className, ".");
}

/********** class Editor **********/

void structEditor :: v_destroy () {
	MelderAudio_stopPlaying (MelderAudio_IMPLICIT);
	/*
	 * The following command must be performed before the shell is destroyed.
	 * Otherwise, we would be forgetting dangling command dialogs here.
	 */
	forget (menus);
	if (d_windowShell) {
		#if gtk
			Melder_assert (GTK_IS_WIDGET (d_windowShell));
			gtk_widget_destroy (GTK_WIDGET (d_windowShell));
		#elif motif
			XtDestroyWidget (d_windowShell);
		#elif ! useCarbon
		#endif
	}
	broadcastDestruction ();
	forget (previousData);
	if (d_ownData) forget (data);
	Editor_Parent :: v_destroy ();
}

void structEditor :: v_info () {
	MelderInfo_writeLine2 (L"Editor type: ", Thing_className (this));
	MelderInfo_writeLine2 (L"Editor name: ", name ? name : L"<no name>");
	time_t today = time (NULL);
	MelderInfo_writeLine2 (L"Date: ", Melder_peekUtf8ToWcs (ctime (& today)));   // includes a newline
	if (data) {
		MelderInfo_writeLine2 (L"Data type: ", data -> classInfo -> className);
		MelderInfo_writeLine2 (L"Data name: ", data -> name);
	}
}

void structEditor :: v_nameChanged () {
	if (name)
		GuiWindow_setTitle (d_windowShell, name);
}

void structEditor :: v_saveData () {
	if (! data) return;
	forget (previousData);
	previousData = Data_copy (data);
}

void structEditor :: v_restoreData () {
	if (data && previousData)
		Thing_swap (data, previousData);
}

static void menu_cb_close (EDITOR_ARGS) {
	EDITOR_IAM (Editor);
	my v_goAway ();
}

static void menu_cb_undo (EDITOR_ARGS) {
	EDITOR_IAM (Editor);
	my v_restoreData ();
	if (wcsnequ (my undoText, L"Undo", 4)) my undoText [0] = 'R', my undoText [1] = 'e';
	else if (wcsnequ (my undoText, L"Redo", 4)) my undoText [0] = 'U', my undoText [1] = 'n';
	else wcscpy (my undoText, L"Undo?");
	#if gtk
		gtk_label_set_label (GTK_LABEL (gtk_bin_get_child (GTK_BIN (my undoButton))), Melder_peekWcsToUtf8 (my undoText));
	#elif motif
		char *text_utf8 = Melder_peekWcsToUtf8 (my undoText);
		XtVaSetValues (my undoButton, XmNlabelString, text_utf8, NULL);
	#endif
	/*
	 * Send a message to myself (e.g., I will redraw myself).
	 */
	my v_dataChanged ();
	/*
	 * Send a message to my boss (e.g., she will notify the others that depend on me).
	 */
	my broadcastDataChanged ();
}

static void menu_cb_searchManual (EDITOR_ARGS) {
	EDITOR_IAM (Editor);
	Melder_search ();
}

static void menu_cb_newScript (EDITOR_ARGS) {
	EDITOR_IAM (Editor);
	(void) ScriptEditor_createFromText (my d_windowParent, me, NULL);
}

static void menu_cb_openScript (EDITOR_ARGS) {
	EDITOR_IAM (Editor);
	autoScriptEditor scriptEditor = ScriptEditor_createFromText (my d_windowParent, me, NULL);
	TextEditor_showOpen (scriptEditor.transfer());
}

void structEditor :: v_createMenuItems_file (EditorMenu menu) {
	(void) menu;
}

void structEditor :: v_createMenuItems_edit (EditorMenu menu) {
	if (data)
		undoButton = EditorMenu_addCommand (menu, L"Cannot undo", GuiMenu_INSENSITIVE + 'Z', menu_cb_undo);
}

static void menu_cb_settingsReport (EDITOR_ARGS) {
	EDITOR_IAM (Editor);
	Thing_info (me);
}

static void menu_cb_info (EDITOR_ARGS) {
	EDITOR_IAM (Editor);
	if (my data) Thing_info (my data);
}

void structEditor :: v_createMenuItems_query (EditorMenu menu) {
	v_createMenuItems_query_info (menu);
}

void structEditor :: v_createMenuItems_query_info (EditorMenu menu) {
	EditorMenu_addCommand (menu, L"Editor info", 0, menu_cb_settingsReport);
	EditorMenu_addCommand (menu, L"Settings report", Editor_HIDDEN, menu_cb_settingsReport);
	if (data) {
		static MelderString title = { 0 };
		MelderString_empty (& title);
		MelderString_append (& title, Thing_className (data), L" info");
		EditorMenu_addCommand (menu, title.string, 0, menu_cb_info);
	}
}

void structEditor :: v_createMenus () {
	EditorMenu menu = Editor_addMenu (this, L"File", 0);
	v_createMenuItems_file (menu);
	if (v_editable ()) {
		menu = Editor_addMenu (this, L"Edit", 0);
		v_createMenuItems_edit (menu);
	}
	if (v_hasQueryMenu ()) {
		menu = Editor_addMenu (this, L"Query", 0);
		v_createMenuItems_query (menu);
	}
}

void structEditor :: v_form_pictureWindow (EditorCommand cmd) {
	LABEL (L"", L"Picture window:")
	BOOLEAN (L"Erase first", 1);
}
void structEditor :: v_ok_pictureWindow (EditorCommand cmd) {
	SET_INTEGER (L"Erase first", preferences.picture.eraseFirst);
}
void structEditor :: v_do_pictureWindow (EditorCommand cmd) {
	preferences.picture.eraseFirst = GET_INTEGER (L"Erase first");
}

void structEditor :: v_form_pictureMargins (EditorCommand cmd) {
	Any radio = 0;
	LABEL (L"", L"Margins:")
	OPTIONMENU_ENUM (L"Write name at top", kEditor_writeNameAtTop, DEFAULT);
}
void structEditor :: v_ok_pictureMargins (EditorCommand cmd) {
	SET_ENUM (L"Write name at top", kEditor_writeNameAtTop, preferences.picture.writeNameAtTop);
}
void structEditor :: v_do_pictureMargins (EditorCommand cmd) {
	preferences.picture.writeNameAtTop = GET_ENUM (kEditor_writeNameAtTop, L"Write name at top");
}

static void gui_window_cb_goAway (I) {
	iam (Editor);
	my v_goAway ();
}

void praat_addCommandsToEditor (Editor me);
void Editor_init (Editor me, GuiObject parent, int x, int y, int width, int height, const wchar *title, Data data) {
	double xmin, ymin, widthmax, heightmax;
	Gui_getWindowPositioningBounds (& xmin, & ymin, & widthmax, & heightmax);
	/*
	 * Negative widths are relative to the whole screen.
	 */
	if (width < 0) width += (int) widthmax;
	if (height < 0) height += (int) heightmax;
	/*
	 * Don't start with a maximized window, because then the user doesn't know what a click on the maximize button means.
	 */
	if (width > (int) widthmax - 100) width = (int) widthmax - 100;
	if (height > (int) heightmax - 100) height = (int) heightmax - 100;
	/*
	 * Make sure that the window has at least a sane size.
	 * Just in case the user made the previous window very small (Praat's FunctionEditor saves the last size),
	 * or the user edited the preferences file (which might save a window size).
	 */
	if (width < 200) width = 200;
	if (height < 150) height = 150;
	/*
	 * Now that the size is right, establish the position.
	 */
	int left, right, top, bottom;
	if (x > 0) {
		/*
		 * Positive x: relative to the left edge of the screen.
		 */
		left = (int) xmin + x;
		right = left + width;
	} else if (x < 0) {
		/*
		 * Negative x: relative to the right edge of the screen.
		 */
		right = (int) xmin + (int) widthmax + x;
		left = right - width;
	} else {
		/*
		 * Zero x: randomize between the left and right edge of the screen.
		 */
		left = NUMrandomInteger ((int) xmin + 4, (int) xmin + (int) widthmax - width - 4);
		right = left + width;
	}
	if (y > 0) {
		/*
		 * Positive y: relative to the top of the screen.
		 */
		top = (int) ymin + y;
		bottom = top + height;
	} else if (y < 0) {
		/*
		 * Negative y: relative to the bottom of the screen.
		 */
		bottom = (int) ymin + (int) heightmax + y;
		top = bottom - height;
	} else {
		/*
		 * Zero y: randomize between the top and bottom of the screen.
		 */
		top = NUMrandomInteger ((int) ymin + 4, (int) ymin + (int) heightmax - height - 4);
		//Melder_casual ("%d %d %d %d", (int) ymin, (int) heightmax, height, top);
		bottom = top + height;
	}
	#if defined (macintoshXXX) || gtk
		top += Machine_getTitleBarHeight ();
		bottom += Machine_getTitleBarHeight ();
	#endif
	my d_windowParent = parent;   // probably praat.topShell
	my d_windowForm = GuiWindow_create (parent, left, top, width, height, title, gui_window_cb_goAway, me, my v_canFullScreen () ? GuiWindow_FULLSCREEN : 0);
	my d_windowShell = GuiObject_parent (my d_windowForm);   // note that GuiObject_parent (my shell) will be NULL!
	//Melder_casual ("my parent %ld my d_windowForm %ld my shell %ld", my parent, my d_windowForm, my shell);
	Thing_setName (me, title);
	my data = data;

	/* Create menus. */

	if (my v_hasMenuBar ()) {
		my menus = Ordered_create ();
		my menuBar = Gui_addMenuBar (my d_windowForm);
		my v_createMenus ();
		EditorMenu helpMenu = Editor_addMenu (me, L"Help", 0);
		my v_createHelpMenuItems (helpMenu);
		EditorMenu_addCommand (helpMenu, L"-- search --", 0, NULL);
		my searchButton = EditorMenu_addCommand (helpMenu, L"Search manual...", 'M', menu_cb_searchManual);
		if (my v_scriptable ()) {
			Editor_addCommand (me, L"File", L"New editor script", 0, menu_cb_newScript);
			Editor_addCommand (me, L"File", L"Open editor script...", 0, menu_cb_openScript);
			Editor_addCommand (me, L"File", L"-- after script --", 0, 0);
		}
		/*
		 * Add the scripted commands.
		 */
		praat_addCommandsToEditor (me);
		Editor_addCommand (me, L"File", L"Close", 'W', menu_cb_close);
		GuiObject_show (my menuBar);
	}

	my v_createChildren ();
	GuiObject_show (my d_windowForm);
}

void Editor_save (Editor me, const wchar *text) {
	my v_saveData ();
	if (! my undoButton) return;
	GuiObject_setSensitive (my undoButton, True);
	swprintf (my undoText, 100, L"Undo %ls", text);
	#if gtk
		gtk_label_set_label (GTK_LABEL (gtk_bin_get_child (GTK_BIN (my undoButton))), Melder_peekWcsToUtf8 (my undoText));
	#elif motif
		char *text_utf8 = Melder_peekWcsToUtf8 (my undoText);
		XtVaSetValues (my undoButton, XmNlabelString, text_utf8, NULL);
	#endif
}

void Editor_openPraatPicture (Editor me) {
	my pictureGraphics = praat_picture_editor_open (preferences.picture.eraseFirst);
}
void Editor_closePraatPicture (Editor me) {
	if (my data != NULL && preferences.picture.writeNameAtTop != kEditor_writeNameAtTop_NO) {
		Graphics_setNumberSignIsBold (my pictureGraphics, false);
		Graphics_setPercentSignIsItalic (my pictureGraphics, false);
		Graphics_setCircumflexIsSuperscript (my pictureGraphics, false);
		Graphics_setUnderscoreIsSubscript (my pictureGraphics, false);
		Graphics_textTop (my pictureGraphics,
			preferences.picture.writeNameAtTop == kEditor_writeNameAtTop_FAR,
			my data -> name);
		Graphics_setNumberSignIsBold (my pictureGraphics, true);
		Graphics_setPercentSignIsItalic (my pictureGraphics, true);
		Graphics_setCircumflexIsSuperscript (my pictureGraphics, true);
		Graphics_setUnderscoreIsSubscript (my pictureGraphics, true);
	}
	praat_picture_editor_close ();
}

/* End of file Editor.cpp */
