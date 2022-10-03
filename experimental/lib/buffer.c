#include "buffer.h"

buffer_t * create_buffer(int32_t size)
{
    buffer_t * self = NULL;
    if (size > 0) {
        self = (buffer_t *) malloc(sizeof(buffer_t));
        MALLOC_TEST(self, __LINE__);
        self->buffer = (void**) malloc(size * sizeof(void*));
        MALLOC_TEST(self->buffer, __LINE__);
        for (int i = 0; i < size; i++) {
            self->buffer[i] = NULL;
        }
        self->size = size;
        self->num_elements = 0;
    }

    return self;
}

int8_t buffer_put(buffer_t * self, void * element) {
    if (self != NULL && element != NULL) {
        self->num_elements++;
        if (self->num_elements >= self->size) {
            self->size *= 2;
            self->buffer = (void**) realloc(self->buffer, self->size * sizeof(void*));
        }
        self->buffer[self->num_elements-1] = element;
        return 0;
    }
    return -1;
}

void * buffer_get(buffer_t * self) {
    void * element = malloc(sizeof(void *));
    element = NULL;
    if (self != NULL && self->num_elements > 0) {
        element = self->buffer[0];
        self->num_elements--;
        int i = 0;
        for (i = 0; i < self->num_elements; i++) {
            self->buffer[i] = self->buffer[i+1];
        }
        self->buffer[i] = NULL;
    }
    return element;
}

void * buffer_peek(buffer_t * self, int32_t index) {
    void * element = NULL;
    if (self != NULL && self->num_elements > index) {
        element = self->buffer[index];
    }
    return element;
}

void buffer_clear(buffer_t * self) {
    if (self != NULL) {
        for (int i = 0; i < self->size; i++) {
            if (self->buffer[i] != NULL) {
                free(self->buffer[i]);
            }
            self->buffer[i] = NULL;
        }
        self->num_elements = 0;
    }
}

void free_buffer(buffer_t * self) {
    if (self == NULL) {
        return;
    }
    buffer_clear(self);
    free(self->buffer);
    free(self);
}