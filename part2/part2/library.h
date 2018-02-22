#include <vector>

#define NUM_ATTR 100
#define LEN_ATTR 10 * sizeof(char)
#define RECORD_SIZE (NUM_ATTR * LEN_ATTR)

#define MIN_DIR_SIZE (sizeof(int) + sizeof(char)) 
#define DIR_ENTRY_SIZE (2*sizeof(int))
#define HEAP_PTR_SIZE (sizeof(int))

#define COL_TUP_SIZE (sizeof(int) + LEN_ATTR)

typedef const char* V;
typedef std::vector<V> Record;

typedef struct {
	void *data;
	int page_size;
	int slot_size;
} Page;

typedef struct {
	FILE *file_ptr;
	int page_size;
	int slot_size;
} Heapfile;

typedef int PageID;

typedef struct {
	int page_id;
	int slot;
} RecordID;

class RecordIterator {
	public:
		RecordIterator(Heapfile *heapfile);
		Record next();
		bool hasNext();
		~RecordIterator();
	private:
		Heapfile *heapfile;
		PageID page_id;
		Page *page;
		int slot;
};

const unsigned char slot_array[] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01, 0};

void load_records(FILE *csv_ptr, std::vector<Record*>* records);

void print_record(Record *r);

int get_file_size(Heapfile *heapfile);

void parse_rid(char *rid, PageID *pid, int *sid);

bool is_page_id_valid(Heapfile *heapfile, PageID pid);

bool is_slot_occupied(Page *page, int slot);

int entries_per_dir(int page_size) ;
/**
 * Compute the number of bytes required to serialize record
 */
int fixed_len_sizeof(Record *record);

/**
 * Serialize the record to a byte array to be stored in buf.
 */
void fixed_len_write(Record *record, void *buf);

/**
 * Deserializes `size` bytes from the buffer, `buf`, and
 * stores the record in `record`.
 */
void fixed_len_read(void *buf, int size, Record *record);

/**
 * Initializes a page using the given slot size
 */
void init_fixed_len_page(Page *page, int page_size, int slot_size);

/**
 * Calculate the maximal number of records that fit in a page
 */
int fixed_len_page_capacity(Page *page);

/**
 * Calculate the free space (number of free slots) in a page
 */
int fixed_len_page_freeslots(Page *page);

/**
 * Add a record to the page
 * Returns:
 *	record slot offset if successful,
 *	-1 if unsucessful (page full)
 */
int add_fixed_len_page(Page *page, Record *r);
int add_fixed_len_page_col(Page *page, Record *r);

/**
 * Write a record into a given slot
 */
void write_fixed_len_page(Page *page, int slot, Record *r);

/**
 * Read a record from the page from a given slot.
 */
void read_fixed_len_page(Page *page, int slot, Record *r);
void read_fixed_len_page_col(Page *page, int slot, Record *r);

/**
 * Initialize a heapfile to use the file and page size given.
 */
void init_heapfile(Heapfile *heapfile, int page_size, FILE *file);

/**
 * Allocate another page in the heapfile. This grows the file by a page. 
 */
PageID alloc_page(Heapfile *heapfile);

/**
 * Read a page into memory
 */
void read_page(Heapfile *heapfile, PageID pid, Page *page);

/**
 * Write a page from memory to disk
 */
void write_page(Page *page, Heapfile *heapfile, PageID pid);

