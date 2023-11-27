#include <gtk/gtk.h>
#include <ctype.h> 

/**
 * A simple text editor created in GTK.
 * Shortcuts:
 *      - Ctrl + S saves the text to a file
 *      - Ctrl + X gives you a dialog to leave without saving
 * TODO:
 *      - CTRL + F function
 *      - Toolbar
*/

static void saveFile(GtkWidget *text_view) {
    GtkWidget *dialog;
    GtkTextBuffer *buffer;
    GtkTextIter start, end;
    gchar *text;
    gint result;

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    dialog = gtk_file_chooser_dialog_new("Save File", NULL,
                                         GTK_FILE_CHOOSER_ACTION_SAVE,
                                         "_Cancel", GTK_RESPONSE_CANCEL,
                                         "_Save", GTK_RESPONSE_ACCEPT,
                                         NULL);
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "untitled.txt");
    result = gtk_dialog_run(GTK_DIALOG(dialog));

    if (result == GTK_RESPONSE_ACCEPT) {
        char *filename;
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        g_file_set_contents(filename, text, -1, NULL);
        g_free(filename);
    }

    g_free(text);
    gtk_widget_destroy(dialog);
}

gboolean on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer text_view) {
    if ((event->state & GDK_CONTROL_MASK) && (event->keyval == GDK_KEY_s || event->keyval == GDK_KEY_S)) {
        saveFile(GTK_WIDGET(text_view));
        return TRUE;
    } else if ((event->state & GDK_CONTROL_MASK) && (event->keyval == GDK_KEY_x || event->keyval == GDK_KEY_X)) {
        GtkWidget *dialog = gtk_dialog_new_with_buttons("Are you sure you want to quit?",
                                                        GTK_WINDOW(widget),
                                                        GTK_DIALOG_MODAL,
                                                        "_Yes", GTK_RESPONSE_YES,
                                                        "_No", GTK_RESPONSE_NO,
                                                        NULL); 
        gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 200);                            
        gtk_widget_show_all(dialog);
        gint result = gtk_dialog_run(GTK_DIALOG(dialog));
        if (result == GTK_RESPONSE_YES) {
            gtk_widget_destroy(widget);
            exit(1);
        } else if (result == GTK_RESPONSE_NO) {
            gtk_widget_destroy(dialog);
        }
    }
    return FALSE;
}

static void newFile() { // todo: just clear textbox :sob:

}

static void openFileButton() { // todo: write open file func and also add a keybind

}

static void saveButton(GtkWidget *widget, gpointer text_view) {
    saveFile(GTK_WIDGET(text_view));
}

static void activate(GtkApplication* app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *box;
    GtkWidget *toolbar;
    GtkWidget *scrolled_window;
    GtkWidget *entry;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Text Editor++");
    gtk_window_set_default_size(GTK_WINDOW(window), 720, 450);

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), box);

    toolbar = gtk_toolbar_new();
    gtk_box_pack_start(GTK_BOX(box), toolbar, FALSE, TRUE, 0);
    gtk_toolbar_set_icon_size(GTK_TOOLBAR(toolbar), GTK_ICON_SIZE_SMALL_TOOLBAR);

    GtkWidget *separator = gtk_separator_new(GTK_ORIENTATION_VERTICAL);
    gtk_widget_set_size_request(separator, 2, 5);
    gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(separator), FALSE, FALSE, 0);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(box), scrolled_window, TRUE, TRUE, 0);

    entry = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(entry), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(entry), TRUE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(entry), TRUE);

    gtk_container_add(GTK_CONTAINER(scrolled_window), entry);

    gtk_widget_show_all(window);
}

int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("org.gtk.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
