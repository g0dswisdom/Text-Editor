#include <gtk/gtk.h>
#include <ctype.h> 

/**
 * gcc `pkg-config --cflags gtk+-3.0` -o app  gui.c `pkg-config --cflags  --libs gtk+-3.0`
 * A simple text editor created in GTK.
 * Shortcuts:
 *      - Ctrl + S saves the text to a file
 *      - Ctrl + X gives you a dialog to leave without saving
 * TODO:
 *      - Syntax highlighting
 *      - CTRL + F function
 *      - Split source into multiple files because it's messy
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

typedef enum {
    TOKEN_KEYWORD, // 0
    TOKEN_IDENTIFIER, // 1
    TOKEN_OPEN_BRACKET, // 2
    TOKEN_CLOSE_BRACKET, // 3
    TOKEN_COMMA, // 4
    TOKEN_EQUAL, // 5
    TOKEN_SEMICOLON, // 6
    TOKEN_NUMBER, // 7
    TOKEN_UNKNOWN // 8
} tokenTypes;

char* identifiers[] = {"int", "char", "float", "double"}; // todo

tokenTypes identify(char* token) {
    for (int i = 0; identifiers[i] != NULL; ++i) {
        if (strcmp(token, identifiers[i]) == 0) {
            return TOKEN_KEYWORD;
        }
    }
    if (strcmp(token, "=") == 0) {
        return TOKEN_EQUAL;
    }

    if (strcmp(token, ";") == 0) {
        return TOKEN_SEMICOLON;
    }

    switch(token[0]) {
        case '[':
            return TOKEN_OPEN_BRACKET;
        case ']':
            return TOKEN_CLOSE_BRACKET;
        case ',':
            return TOKEN_COMMA;
    }

    int len = strlen(token);
    int isNumber = 1;
    int hasDot = 0;
    for (int i = 0; i < len; ++i) {
        if (!isdigit(token[i])) {
            if (token[i] == '.' && !hasDot) {
                hasDot = 1;
            } else {
                isNumber = 0;
                break;
            }
        }
    }
    if (isNumber) {
        return TOKEN_NUMBER;
    }
    if (isalpha(token[0])) {
        return TOKEN_IDENTIFIER;
    }

    return TOKEN_UNKNOWN;
}

static void syntaxHighlighter(GtkWidget *text_view) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    GtkTextIter start, end;
    gchar *text;

    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    text = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    GRegex *regex = g_regex_new("\\b(?:int|char|float|double)\\b|\\b[0-9]+(?:\\.[0-9]*)?\\b|\\S", 0, 0, NULL);
    GMatchInfo *match_info;

    g_regex_match(regex, text, 0, &match_info);

    while (g_match_info_matches(match_info)) {
        gchar *token = g_match_info_fetch(match_info, 0);
        tokenTypes type = identify(token);
        switch (type) {
            case TOKEN_KEYWORD:
                g_print("Got keyword: %s\n", token);
                break;
            case TOKEN_IDENTIFIER:
                g_print("Got identifier: %s\n", token);
                break;
            case TOKEN_NUMBER:
                g_print("Got number: %s\n", token);
                break;
            case TOKEN_EQUAL:
                g_print("Got equal: %s\n", token);
                break;
        }
        g_free(token);
        g_match_info_next(match_info, NULL);
    }

    g_match_info_free(match_info);
    g_regex_unref(regex);
    g_free(text);
}

void on_buffer_changed(GtkTextBuffer *buffer, gpointer user_data) {
    GtkWidget *text_view = GTK_WIDGET(user_data);
    syntaxHighlighter(text_view); 
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

static void activate(GtkApplication* app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *scrolled_window;
    GtkWidget *entry;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Text Editor++");
    gtk_window_set_default_size(GTK_WINDOW(window), 720, 450);

    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(window), scrolled_window);

    entry = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(entry), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(entry), TRUE); 
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(entry), TRUE);

    gtk_container_add(GTK_CONTAINER(scrolled_window), entry);

    g_signal_connect(G_OBJECT(window), "key-press-event", G_CALLBACK(on_key_press), entry);

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(entry));
    g_signal_connect(buffer, "changed", G_CALLBACK(on_buffer_changed), entry);
    syntaxHighlighter(entry);

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
