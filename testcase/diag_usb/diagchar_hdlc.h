#ifndef DIAGCHAR_HDLC
#define DIAGCHAR_HDLC

typedef		__u8		uint8_t;
typedef		__u16		uint16_t;
typedef		__u32		uint32_t;

enum diag_send_state_enum_type {
	DIAG_STATE_START,
	DIAG_STATE_BUSY,
	DIAG_STATE_CRC1,
	DIAG_STATE_CRC2,
	DIAG_STATE_TERM,
	DIAG_STATE_COMPLETE
};

struct diag_send_desc_type {
	const void *pkt;
	const void *last;	/* Address of last byte to send. */
	enum diag_send_state_enum_type state;
	unsigned char terminate;	/* True if this fragment
					   terminates the packet */
};

struct diag_hdlc_dest_type {
	void *dest;
	void *dest_last;
	/* Below: internal use only */
	uint16_t crc;
};

struct diag_hdlc_decode_type {
	uint8_t *src_ptr;
	unsigned int src_idx;
	unsigned int src_size;
	uint8_t *dest_ptr;
	unsigned int dest_idx;
	unsigned int dest_size;
	int escaping;

};

int diag_hdlc_encode(struct diag_send_desc_type *src_desc,
		      struct diag_hdlc_dest_type *enc);

int diag_hdlc_decode(struct diag_hdlc_decode_type *hdlc);

int crc_check(uint8_t *buf, uint16_t len);

int diag_usb_encode(void *in, int in_len, void *out, int out_len);

int diag_usb_decode(void *in, int in_len, void *out, int out_len);

#define ESC_CHAR     0x7D
#define ESC_MASK     0x20

#define CONTROL_CHAR	0x7E
#define CRC_16_L_SEED           0xffff

#endif
