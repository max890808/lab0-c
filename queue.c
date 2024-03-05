#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (head) {
        INIT_LIST_HEAD(head);
        return head;
    }
    return NULL;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;

    element_t *entry, *safe;

    list_for_each_entry_safe (entry, safe, head, list) {
        q_release_element(entry);
    }

    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head || !s)
        return 0;

    element_t *element = (element_t *) malloc(sizeof(element_t));

    if (!element)
        return 0;

    element->value = malloc((strlen(s) + 1) * sizeof(char));
    if (!element->value) {
        free(element);
        return 0;
    }

    strncpy(element->value, s, strlen(s));
    element->value[strlen(s)] = '\0';
    list_add(&element->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head || !s)
        return 0;

    element_t *element = (element_t *) malloc(sizeof(element_t));

    if (!element)
        return 0;

    element->value = malloc((strlen(s) + 1) * sizeof(char));
    if (!element->value) {
        free(element);
        return 0;
    }

    strncpy(element->value, s, strlen(s));
    element->value[strlen(s)] = '\0';
    list_add_tail(&element->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *first_element = list_first_entry(head, element_t, list);

    list_del(&first_element->list);

    if (sp) {
        strncpy(sp, first_element->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return first_element;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *last_element = list_last_entry(head, element_t, list);

    list_del(&last_element->list);

    if (sp) {
        strncpy(sp, last_element->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    return last_element;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}


/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/
    if (!head || list_empty(head))
        return false;

    struct list_head **indir = &head->next;
    for (struct list_head *fast = head->next;
         fast != head && fast->next != head; fast = fast->next->next) {
        indir = &(*indir)->next;
    }

    element_t *mid_element = list_entry(*indir, element_t, list);
    list_del(*indir);
    q_release_element(mid_element);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head || list_empty(head))
        return false;
    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list) {
        if (&safe->list != head && strcmp(entry->value, safe->value) == 0) {
            while (&safe->list != head &&
                   strcmp(entry->value, safe->value) == 0) {
                struct list_head *next = safe->list.next;
                q_release_element(safe);
                entry->list.next = next;
                next->prev = &entry->list;
                safe = list_entry(next, element_t, list);
            }
            struct list_head *prev = entry->list.prev;
            q_release_element(entry);
            prev->next = &safe->list;
            safe->list.prev = prev;
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    struct list_head **indir = &head->next;
    for (; *indir != head && (*indir)->next != head;
         indir = &(*indir)->next->next) {
        list_move(*indir, (*indir)->next);
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    struct list_head **indir = &head->next, *next = NULL;
    for (; *indir != head; indir = &(*indir)->prev) {
        next = (*indir)->next;
        (*indir)->next = (*indir)->prev;
        (*indir)->prev = next;
    }
    next = head->next;
    head->next = head->prev;
    head->prev = next;
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    int i = 0;
    struct list_head tmp_queue, *tmp_head = head, *safe, *node;
    INIT_LIST_HEAD(&tmp_queue);
    list_for_each_safe (node, safe, head) {
        i++;
        if (i == k) {
            list_cut_position(&tmp_queue, tmp_head, node);
            q_reverse(&tmp_queue);
            list_splice_init(&tmp_queue, tmp_head);
            tmp_head = safe->prev;
            i = 0;
        }
    }
}

void merge(struct list_head *left,
           struct list_head *right,
           struct list_head *head,
           bool descend)
{
    struct list_head *ptr = head;
    while (!list_empty(left) && !list_empty(right)) {
        element_t *element_l = list_entry(left->next, element_t, list);
        element_t *element_r = list_entry(right->next, element_t, list);

        if (descend) {
            if (strcmp(element_l->value, element_r->value) < 0) {
                list_move(right->next, ptr);
                ptr = ptr->next;
            } else {
                list_move(left->next, ptr);
                ptr = ptr->next;
            }
        } else {
            if (strcmp(element_l->value, element_r->value) < 0) {
                list_move(left->next, ptr);
                ptr = ptr->next;
            } else {
                list_move(right->next, ptr);
                ptr = ptr->next;
            }
        }
    }
    struct list_head *tmp = list_empty(left) ? right : left;
    list_splice_tail(tmp, ptr->next);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;

    struct list_head *slow = head->next, *fast = head->next->next;
    while (fast != head && fast->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }

    LIST_HEAD(left);
    LIST_HEAD(right);

    list_cut_position(&left, head, slow);
    list_splice_init(head, &right);

    q_sort(&left, descend);
    q_sort(&right, descend);
    merge(&left, &right, head, descend);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;
    element_t *tmp_element, *node_element;
    struct list_head *node, *safe;
    int i = 0;
    list_for_each_safe (node, safe, head) {
        struct list_head *tmp = head->next, *tmp_next = tmp;
        for (; tmp != node; tmp = tmp_next) {
            tmp_element = list_entry(tmp, element_t, list);
            node_element = list_entry(node, element_t, list);
            tmp_next = tmp->next;
            if (strcmp(tmp_element->value, node_element->value) > 0) {
                list_del(tmp);
                q_release_element(tmp_element);
            }
        }
    }
    struct list_head *node1;
    list_for_each (node1, head) {
        i++;
    }
    return i;
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    // https://leetcode.com/problems/remove-nodes-from-linked-list/
    if (!head || list_empty(head))
        return 0;
    element_t *tmp_element, *node_element;
    struct list_head *node, *safe;
    int i = 0;
    list_for_each_safe (node, safe, head) {
        struct list_head *tmp = head->next, *tmp_next = tmp;
        for (; tmp != node; tmp = tmp_next) {
            tmp_element = list_entry(tmp, element_t, list);
            node_element = list_entry(node, element_t, list);
            tmp_next = tmp->next;
            if (strcmp(tmp_element->value, node_element->value) < 0) {
                list_del(tmp);
                q_release_element(tmp_element);
            }
        }
    }
    struct list_head *node1;
    list_for_each (node1, head) {
        i++;
    }
    return i;
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    if (!head || list_empty(head))
        return 0;
    queue_contex_t *tmp = list_entry(head->next, queue_contex_t, chain);
    struct list_head *ptr = head->next->next;
    while (ptr != head) {
        queue_contex_t *tmp2 = list_entry(ptr, queue_contex_t, chain);
        list_splice_init(tmp2->q, tmp->q);
        ptr = ptr->next;
    }
    q_sort(tmp->q, descend);

    return q_size(tmp->q);
}
