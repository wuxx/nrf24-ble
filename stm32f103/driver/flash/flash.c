#include "flash.h"
#include "mmio.h"
#include "log.h"
#include "common.h"

static uint8_t page[FLASH_PAGE_SIZE];

int32_t flash_write(uint32_t addr, void *buf, uint32_t size)
{
    uint32_t i, x;
    int32_t status;

    uint32_t *wpage;
    uint8_t  *bpage;

    uint32_t wdata;

    uint8_t  *bbuf;
    uint32_t offset, page_start, page_num;
	
		uint32_t head_size, body_size, tail_size;
	
    PRINT_EMG("%s-%d 0x%08x 0x%08x 0x%08x 0x%08x\n", __func__, __LINE__, addr, buf, size, page);

		if (size == 0) {
			PRINT_ERR("size must > 0\n");
			return -1;
		}

#if 0
    if ((size % 4) != 0) {
        PRINT_ERR("size must align by word (%x)\n", size);
        return -1;     
    }
#endif

    if (!((addr >= FLASH_BASE) && ((addr + size) <= (FLASH_BASE + FLASH_SIZE)))) {
        PRINT_ERR("illegal addr interval [%x, %x], must belong to [%x, %x]\n", 
                addr, addr + size, FLASH_BASE, FLASH_BASE + FLASH_SIZE);
        return -1;
    }

    FLASH_Unlock();
		FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);	

	/* we always cut the addr interval in 3 part: head, body, tail */
	/* TODO: DRY. there is too much duplicate code */

	/* head */
	if ((offset = PAGE_OFFSET(addr)) != 0) {
		PRINT_EMG("head offest 0x%08x\n", offset);
		/* read */
		wpage = (uint32_t *)&page[0];
		bpage = (uint8_t  *)&page[0];
		bbuf  = (uint8_t  *)buf;

		for(i = 0; i < (FLASH_PAGE_SIZE / 4); i++) {
			wpage[i] = readl(PAGE_BASE(addr) + i * 4);
		}
		//PRINT_EMG("%s-%d %x\n", __func__, __LINE__, PAGE_BASE(addr));	
		/* erase */
		status = FLASH_ErasePage(PAGE_BASE(addr));
#if 0
		PRINT_EMG("earse page 0x%08x return \n", PAGE_BASE(addr));
		return;
#endif
		/* merge */
		head_size = size <= (FLASH_PAGE_SIZE - offset) ? size : FLASH_PAGE_SIZE - offset;
		for(i = 0; i < head_size; i++) {
			bpage[offset + i] = bbuf[i];
		}

		/* write back */
		for (i = 0; i < (FLASH_PAGE_SIZE / 4); i++) {
			status = FLASH_ProgramWord(PAGE_BASE(addr) + i * 4, wpage[i]);
		
			if (status != FLASH_COMPLETE) {
				PRINT_ERR("program fail [0x%08x]:0x%08x (status %d)\n", 
					PAGE_BASE(addr) + i * 4,  wpage[i], status);
				goto error;
			}

			if ((wdata = readl(PAGE_BASE(addr) + i * 4)) != wpage[i]) {
				PRINT_ERR("%d: check fail [0x%08x]:0x%08x (expect 0x%08x:0x%08x)\n", 
					__LINE__, PAGE_BASE(addr) + i * 4,  wdata, &wpage[i], wpage[i]);
				goto error;
			}
		}

		if (PAGE_BASE(addr) == PAGE_BASE(addr + size - 1)) {
			return 0;
		}
	}

	/* tail */
	if ((offset = PAGE_OFFSET(addr + size)) != 0) {
		PRINT_EMG("tail offset 0x%08x\n", offset);
		/* read */
		wpage = (uint32_t *)&page[0];
		bpage = (uint8_t  *)&page[0];
		bbuf  = (uint8_t  *)buf + size - offset;
		
		for(i = 0; i < (FLASH_PAGE_SIZE / 4); i++) {
			wpage[i] = readl(PAGE_BASE(addr + size) + i * 4);
		}

		//PRINT_EMG("%s-%d %x\n", __func__, __LINE__, PAGE_BASE(addr + size));	
		/* erase */
		status = FLASH_ErasePage(PAGE_BASE(addr + size));

		/* merge */
		for(i = 0; i < (offset); i++) {
			bpage[i] = bbuf[i];
		}
		
		for (i = 0; i < (FLASH_PAGE_SIZE / 4); i++) {
			status = FLASH_ProgramWord(PAGE_BASE(addr + size) + i * 4, wpage[i]);
		
			if (status != FLASH_COMPLETE) {
				PRINT_ERR("%d: program fail [%x]:%x (status %x)\n", 
					__LINE__, PAGE_BASE(addr + size) + i * 4,  wpage[i], status);
				goto error;
			}
		
			if ((wdata = readl(PAGE_BASE(addr + size) + i * 4)) != wpage[i]) {
				PRINT_ERR("%d: check fail [0x%08x]:0x%08x (expect 0x%08x:0x%08x)\n", 
					__LINE__, PAGE_BASE(addr + size) + i * 4,  wdata, &wpage[i], wpage[i]);
				goto error;
			}
		}

	}

	page_start = PAGE_OFFSET(addr) == 0 ? addr : addr + (FLASH_PAGE_SIZE - PAGE_OFFSET(addr));
	page_num   = (PAGE_BASE(addr + size) - page_start) / FLASH_PAGE_SIZE;
	wpage      = PAGE_OFFSET(addr) == 0 ? (uint32_t *)(buf) : (uint32_t *)((uint32_t)buf + FLASH_PAGE_SIZE - PAGE_OFFSET(addr));
	/* body */
	PRINT_EMG("page_num: %d\n", page_num);
	for(x = 0; x < page_num; x++) {
		PRINT_EMG("body %d\n", x);
		/* erase */
		//PRINT_EMG("erase1 0x%08x\n", page_start + x * FLASH_PAGE_SIZE);
		status = FLASH_ErasePage(page_start + x * FLASH_PAGE_SIZE);
		//PRINT_EMG("erase2 0x%08x\n", page_start + x * FLASH_PAGE_SIZE);
		for(i = 0; i < (FLASH_PAGE_SIZE / 4); i++) {
			//PRINT_EMG("program1: 0x%08x: 0x%08x\n", page_start + x * FLASH_PAGE_SIZE + i * 4, &wpage[(x * FLASH_PAGE_SIZE) / 4 + i]);
			status = FLASH_ProgramWord(page_start + x * FLASH_PAGE_SIZE + i * 4, wpage[(x * FLASH_PAGE_SIZE) / 4 + i]);
			//PRINT_EMG("program2: 0x%08x: 0x%08x\n", page_start + x * FLASH_PAGE_SIZE + i * 4, &wpage[(x * FLASH_PAGE_SIZE) / 4 + i]);
			if (status != FLASH_COMPLETE) {
				PRINT_ERR("program fail [%x]:%x (status %x)\n", 
					page_start + x * FLASH_PAGE_SIZE + i * 4,  wpage[x * FLASH_PAGE_SIZE + i], status);
				goto error;
			}
		
			//PRINT_EMG("%d: %x %x\n", __LINE__, page_start + x * FLASH_PAGE_SIZE + i * 4, wpage[x * FLASH_PAGE_SIZE + i]);
			
			if ((wdata = readl(page_start + x * FLASH_PAGE_SIZE + i * 4)) != wpage[(x * FLASH_PAGE_SIZE) / 4 + i]) {
				PRINT_ERR("%d: check fail [0x%08x]:0x%08x (expect 0x%08x:0x%08x)\n", 
					__LINE__, page_start + x * FLASH_PAGE_SIZE + i * 4,  wdata, &wpage[(x * FLASH_PAGE_SIZE) / 4 + i], wpage[(x * FLASH_PAGE_SIZE) / 4 + i]);
				goto error;
			}

		}
	}

    FLASH_Lock();

    return 0;
	
error:
 	FLASH_Lock();
	return -1;
}

int32_t flash_read(uint32_t addr, void *buf, uint32_t size)
{
    uint32_t i, word_num;
    uint32_t *b = buf;

    if ((size % 4) != 0) {
        PRINT_ERR("size must align by word (%x)\n", size);
        return -1;     
    }

    if (!((addr >= FLASH_BASE) && ((addr + size) < (FLASH_BASE + FLASH_SIZE)))) {
        PRINT_ERR("illegal interval [%x, %x], must belong to [%x, %x]\n", 
                addr, addr + size, FLASH_BASE, FLASH_BASE + FLASH_SIZE);
        return -1;    
    }

    word_num = size / 4;
    for (i = 0; i < word_num; i++) {
        b[i] = readl(addr + i * 4);
    }
    
	return 0;
}
