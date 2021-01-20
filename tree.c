#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <dirent.h>

#define BUFFSIZE 256
#define EMPTY_LIST NULL

#define INDENT       "│   "
#define INDENT_EMPTY "    "
#define PREFIX       "├── "
#define PREFIX_END   "└── "

typedef enum{false=0, true=1} bool;

typedef struct node_s node_t;
typedef union {
    int integer;
    struct {
        char * name;
        node_t * subdirs;
    };
} listable_t;
struct node_s {
    listable_t head;
    node_t * tail;
};

void build_tree(char *, int, node_t **);
void print_tree(node_t *, char **);
void list_append(node_t **, listable_t);
int list_length(node_t **);
void list_sort(node_t **, int (*)(listable_t, listable_t));
void list_free(node_t **, void (*)(listable_t));
int cmp_int(listable_t, listable_t);
int cmp_str(listable_t, listable_t);

int dircount;
int filecount;

bool showAll;

int main(int argc, char ** argv) {
    char dir[BUFFSIZE];
    dircount = -1;
    filecount = 0;

    // defaults
    strcpy(dir, ".");
    showAll = false;

    // parse arguments
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-a") == 0) showAll = true;
        else if (strcmp(argv[i], "--help") == 0) {
            printf("usage: %s [OPTIONS] [dir]\n", argv[0]);
            printf("possible options:\n");
            printf("-a\tshow all files (hidden ones)\n");
            printf("--help\tshow this help page\n");
            exit(0);
        } else strncpy(dir, argv[i], BUFFSIZE);
    }

    node_t * root = EMPTY_LIST;
    build_tree(dir, BUFFSIZE, &root);
    list_sort(&root, cmp_str);

    if (dircount < 0) {
        fprintf(stderr, "%s requires a directory as argument, not a file\n", argv[0]);
        exit(1);
    }

    char ** prefixes = (char **) malloc(BUFFSIZE * sizeof(char *));
    assert(prefixes);
    for (int i = 0; i < BUFFSIZE; ++i) prefixes[i] = NULL;

    if (!root || root->head.subdirs == NULL) {
        fprintf(stderr, "%s is an empty directory", dir);
        exit(1);
    }

    printf("%s\n", root->head.name);
    print_tree(root->head.subdirs, prefixes);

    printf("\n%d director%s, %d file%s\n", dircount, (dircount != 1) ? "ies" : "y", filecount, (filecount != 1) ? "s" : "");

    return 0;
}

void build_tree(char * path, int length, node_t ** list) {
    listable_t file;

    int pathlen = strlen(path);

    // save the name
    int namelength;
    { // name
        namelength = 0;
        char * end = path + pathlen;
        while (--end >= path && *end != '/') namelength++;

        file.name = malloc((namelength + 2) * sizeof(char)); // +2: null-char, maybe '/' if directory
        strncpy(file.name, path + pathlen - namelength, namelength);
        file.name[namelength] = '\0';
        file.name[namelength+1] = '\0';
    }
    file.subdirs = EMPTY_LIST;

    // actually open
    DIR * dir = opendir(path);
    if (dir) { // if is a directory
        ++dircount;

        file.name[namelength] = '/';

        struct dirent * curr;
        while (true) {
            curr = readdir(dir);
            if (curr == NULL) break;
            char * name = curr->d_name;

            // if it shall be displayed (without -a option: does not start with .; with -a option: is not . or ..)
            if ((!showAll && name[0] != '.') || (showAll && strcmp(name, ".") && strcmp(name, ".."))) {

                path[pathlen] = '/';
                strncpy(path + pathlen + 1, name, length-pathlen-1);
                path[length-1] = '\0'; // better safe

                build_tree(path, length, &file.subdirs);
                list_sort(&file.subdirs, cmp_str);
            }
        }
    } else { // is a file (or non existent)
        ++filecount;
    }
    list_append(list, file);
}

void print_tree(node_t * tree, char ** prefixes) {
    char * name;
    node_t * subdirs;
    while (tree != EMPTY_LIST) {
        name = tree->head.name;
        subdirs = tree->head.subdirs;

        int i;
        for (i = 0; prefixes[i] != NULL; ++i) {
            printf("%s", prefixes[i]);
        }

        printf("%s%s\n", tree->tail ? PREFIX : PREFIX_END, name);

        if (subdirs) {
            assert(i < BUFFSIZE);
            prefixes[i] = tree->tail ? INDENT : INDENT_EMPTY;
            print_tree(subdirs, prefixes);
            prefixes[i] = NULL;
        }

        tree = tree->tail;
    }
}

void list_append(node_t ** list, listable_t data) {
    assert(list);
    if (*list) {
        list_append(&(*list)->tail, data);
    } else {
        *list = (node_t *) malloc(sizeof(node_t));
        (*list)->head = data;
        (*list)->tail = EMPTY_LIST;
    }
}

int list_length(node_t ** list) {
    assert(list);
    if (*list) {
        return 1 + list_length(&(*list)->tail);
    } else {
        return 0;
    }
}

void list_sort(node_t ** list, int (*cmp)(listable_t, listable_t)) {
    assert(list);

    if (*list) {
        listable_t pivot = (*list)->head;
        node_t * left = EMPTY_LIST;
        node_t * right = EMPTY_LIST;
        node_t * iterator = (*list)->tail;

        while (iterator != EMPTY_LIST) {
            if (cmp(iterator->head, pivot) < 0) {
                list_append(&left, iterator->head);
            } else {
                list_append(&right, iterator->head);
            }

            iterator = iterator->tail;
        }
        list_free(list, NULL);

        list_sort(&left, cmp);
        list_sort(&right, cmp);

        iterator = left;
        while (iterator != EMPTY_LIST) {
            list_append(list, iterator->head);
            iterator = iterator->tail;
        }
        list_free(&left, NULL);

        list_append(list, pivot);

        iterator = right;
        while (iterator != EMPTY_LIST) {
            list_append(list, iterator->head);
            iterator = iterator->tail;
        }
        list_free(&right, NULL);
    }
}

void list_free(node_t ** list, void (*f)(listable_t)) {
    assert(list);
    if (*list) {
        list_free(&(*list)->tail, f);
        if (f) f((*list)->head);
        free(*list);
        *list = EMPTY_LIST;
    }
}

void free_f(listable_t elem) {
    free(elem.name);
    list_free(&elem.subdirs, free_f);
}

int cmp_int(listable_t a, listable_t b) {
    return a.integer - b.integer;
}

int cmp_str(listable_t a, listable_t b) {
    return strcmp(a.name, b.name);
}
