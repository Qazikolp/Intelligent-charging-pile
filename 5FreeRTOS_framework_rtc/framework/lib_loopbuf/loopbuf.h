

typedef struct {
    unsigned char* buffer;
    unsigned int size;
    unsigned int head;
    unsigned int tail;
} loopbuf_t;


extern loopbuf_t* loopbuf_init( uint32_t size);
extern void loopbuf_free(loopbuf_t* lb);
extern uint32_t loopbuf_write(loopbuf_t* lb, const uint8_t* data, uint32_t n);
extern uint32_t loopbuf_read(loopbuf_t* lb, uint8_t* data, uint32_t n);
extern void loopbuf_reset( loopbuf_t* lb );

