#include <assert.h>
#include <elf.h>
#include <stdint.h>
#include <string.h>

#include <sys/user.h>

#include "common.h"

#include "elf_gen.h"

const size_t gen_elf(
		     uint8_t **out,
		     const unsigned long start,
		     const uint8_t *const code,
		     const size_t code_sz)
{
  /* We give the elf header and phdr an entire page, because the elf loader can
   * only map the file at PAGE_SIZE offsets. So our file will look like this 
   * for an invocation with some code and 2 data segments.
   *
   * +----------+
   * | 1st page |
   * | ehdr     |
   * | phdr     |
   * |----------|
   * | 2nd page |  <-- Ian says: actually code_sz / PAGE_SZ [ + 1] pages.
   * | code     |      But for now, to keep things simple, we will put 
   * |          |      in an assert to ensure it is exactly one page.
   * |----------|
   * | 3rd page |
   * | data     |
   * |          |
   * |----------|
   * | shdr     |
   * |          |
   * |   str_tbl|
   * +----------+
   *
   *
   */

  assert(code_sz <= PAGE_SIZE);

  uint8_t const _str_tbl[] =  { 0, 
				'.', 't', 'e', 'x', 't', 0, 
				'.', 'd', 'a', 't', 'a', 0, 
				'.', 's', 'h', 's', 't', 'r', 't', 'a', 'b', 0 };

  
  const size_t str_tbl_sz = sizeof(_str_tbl);

  const int nshdr  =  4;  //one initial, one text, one data, and one section header string table

  const size_t pg_align_dist = start - (start & ~0xffff);
  const size_t pad_sz = PAGE_SIZE - (PAGE_SIZE % code_sz);
  const size_t sz = PAGE_SIZE + pg_align_dist + code_sz + pad_sz;

  const size_t d_sz =  PAGE_SIZE;

  uint8_t *const e = xmalloc(4 * PAGE_SIZE);

  mem_assign(e, sz, TRAP, TRAP_SZ);

  Elf64_Ehdr *const ehdr = (Elf64_Ehdr *) e;
  
  const size_t shdr_offset = 3 * PAGE_SIZE;
	
  ehdr->e_ident[0] = ELFMAG0;              // 0x7f
  ehdr->e_ident[1] = ELFMAG1;              // 'E'
  ehdr->e_ident[2] = ELFMAG2;              // 'L'
  ehdr->e_ident[3] = ELFMAG3;              // 'F'
  ehdr->e_ident[4] = ELFCLASS64;           // Class 32/64 (64 bit here)
  ehdr->e_ident[5] = ELFDATA2LSB;          // Byte order (little endian)
  ehdr->e_ident[6] = EV_CURRENT;           // Elf spec version
  ehdr->e_ident[7] = ELFOSABI_NONE;        // ABI (UNIX System V ABI)
  ehdr->e_ident[9] = 0;                    // Padding
  ehdr->e_type = ET_EXEC;                  // Type (executable)
  ehdr->e_machine = EM_X86_64;             // Architecture (AMD x86_64)
  ehdr->e_version = EV_CURRENT;            // File version
  ehdr->e_entry = start;                   // entry point
  ehdr->e_phoff = sizeof(Elf64_Ehdr);      // offset to program header
  ehdr->e_shoff = shdr_offset;             // Section header offset XXX
  ehdr->e_flags = 0;                       // Flags
  ehdr->e_ehsize = sizeof(Elf64_Ehdr);     // Elf header size in bytes
  ehdr->e_phentsize = sizeof(Elf64_Phdr);  // Size of one entry in the program header table
  ehdr->e_phnum = 2;                       // Number of entries in the program header table
  ehdr->e_shentsize = sizeof(Elf64_Shdr);  // Size of one entry in the section header table
  ehdr->e_shnum = nshdr;                   // Number of entries in the section header table
  ehdr->e_shstrndx = nshdr - 1;            // Section header name string index (should be: SHN_UNDEF)

  Elf64_Phdr *const phdr = (Elf64_Phdr *) ((uint8_t *) e + sizeof(Elf64_Ehdr));

  phdr[0].p_type = PT_LOAD;
  phdr[0].p_flags = PF_X | PF_R;
  phdr[0].p_offset = PAGE_SIZE;
  phdr[0].p_vaddr = start - pg_align_dist;
  phdr[0].p_paddr = 0;
  phdr[0].p_filesz = code_sz + pg_align_dist; 
  phdr[0].p_memsz = code_sz + pg_align_dist;
  phdr[0].p_align = 0x4;

  phdr[1].p_type = PT_LOAD;
  phdr[1].p_flags = PF_W | PF_R;
  phdr[1].p_offset = 2 * PAGE_SIZE;
  phdr[1].p_vaddr = start + PAGE_SIZE;
  phdr[1].p_paddr = 0;
  phdr[1].p_filesz = PAGE_SIZE; 
  phdr[1].p_memsz = PAGE_SIZE;
  phdr[1].p_align = 0x8;

  uint8_t *const text = (uint8_t *) e + PAGE_SIZE + pg_align_dist;
  memcpy(text, code, code_sz);

  uint8_t *const data = (uint8_t *) e  + 2 * PAGE_SIZE;

  memset(data, 1, d_sz);

  Elf64_Shdr *const shdr = ( Elf64_Shdr *)(e + (3 * PAGE_SIZE)); 

  memset(shdr, 0, PAGE_SIZE);

  uint8_t *const str_tbl = (uint8_t *)(e + (4 * PAGE_SIZE) - str_tbl_sz - 1);

  memcpy(str_tbl, _str_tbl, str_tbl_sz);

  //zero-th section header
  shdr[0].sh_name = 0;
  shdr[0].sh_type = SHT_NULL;
  shdr[0].sh_flags = 0;
  shdr[0].sh_addr = 0;            
  shdr[0].sh_offset = 0;
  shdr[0].sh_size = 0;
  shdr[0].sh_link = SHN_UNDEF;
  shdr[0].sh_info = 0;
  shdr[0].sh_addralign = 0;
  shdr[0].sh_entsize = 0;

  //text section
  shdr[1].sh_name = 1;              //index into the str_tbl
  shdr[1].sh_type = SHT_PROGBITS;
  shdr[1].sh_flags = SHF_EXECINSTR | SHF_ALLOC;
  shdr[1].sh_addr = start;          //address where the section *will* reside 
  shdr[1].sh_offset = PAGE_SIZE;    //where this section starts in the file
  shdr[1].sh_size = code_sz;        //size in bytes
  shdr[1].sh_link = SHN_UNDEF;
  shdr[1].sh_info = 0;
  shdr[1].sh_addralign = 16;        
  shdr[1].sh_entsize = 0;


  //data section
  shdr[2].sh_name = 7;                 //index into the str_tbl
  shdr[2].sh_type = SHT_PROGBITS;
  shdr[2].sh_flags = SHF_WRITE | SHF_ALLOC;
  shdr[2].sh_addr = start + PAGE_SIZE; //address where the section *will* reside
  shdr[2].sh_offset = 2 * PAGE_SIZE;   //where this section starts in the file
  shdr[2].sh_size = PAGE_SIZE;         //size in bytes
  shdr[2].sh_link = SHN_UNDEF;
  shdr[2].sh_info = 0;
  shdr[2].sh_addralign = 32;           //no alignment requirements
  shdr[2].sh_entsize = 0;

  //section header string table
  shdr[3].sh_name = 13;                 //index into the str_tbl
  shdr[3].sh_type = SHT_STRTAB;
  shdr[3].sh_flags = 0;
  shdr[3].sh_addr = 0;                 //address where the section *will* reside
  shdr[3].sh_offset = 4 * PAGE_SIZE - str_tbl_sz - 1;   //where this section starts in the file
  shdr[3].sh_size = str_tbl_sz;        //size in bytes
  shdr[3].sh_link = SHN_UNDEF;
  shdr[3].sh_info = 0;
  shdr[3].sh_addralign = 1;            //no alignment requirements
  shdr[3].sh_entsize = 0;

  *out = e;

  return 4 * PAGE_SIZE;
}
