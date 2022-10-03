#include "link.h"

struct link {
    int16_t src_node;
    int16_t dst_node;
    buffer_t * fifo;
};

link_t create_link(int16_t src_node, int16_t dst_node, int32_t capacity)
{
    link_t self = (link_t) malloc(sizeof(struct link));
    MALLOC_TEST(self, __LINE__);
    self->src_node = src_node;
    self->dst_node = dst_node;
    self->fifo = create_buffer(capacity);
    return self;
}

void link_enqueue(link_t self, void* element)
{
    NULL_TEST(self, __LINE__);
    assert(buffer_put(self->fifo, element) != -1);
}

void* link_dequeue(link_t self)
{
    NULL_TEST(self, __LINE__);
    return buffer_get(self->fifo);
}

void* link_peek(link_t self)
{
    NULL_TEST(self, __LINE__);
    return buffer_peek(self->fifo, 0);
}

void free_link(link_t self)
{
    if (self != NULL) {
        free_buffer(self->fifo);
        free(self);
    }
}
