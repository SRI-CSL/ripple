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
   * | shdr     |  <-- Ian says: section headers come at the end of the file.
   * | shdr     |
   * |----------|
   * | 2nd page |
   * | code     |
   * |----------|
   * | 3rd page |
   * | data 1   |
   * |----------|
   * | 4th page |
   * | data 2   |
   * +----------+
   *
   * TODO add data section, section headers
   *
   * Ian says:  not seeing the two pages of data...
   *
   */

  const int nsh  =  3;  //one initial, one text and one data
  const sh_sz =  nsh * sizeof(Elf64_Shdr);
  const foot_pad_sz = PAGE_SIZE - (PAGE_SIZE % section_headers_sz);
  const footsz = sh_sz + foot_pad_sz;

  const size_t pg_align_dist = start - (start & ~0xffff);
  const size_t pad_sz = PAGE_SIZE - (PAGE_SIZE % code_sz);
  const size_t sz = PAGE_SIZE + pg_align_dist + code_sz + pad_sz;

  uint8_t *const e = xmalloc(sz);
  
  mem_assign(e, sz, TRAP, TRAP_SZ);

  Elf64_Ehdr *const ehdr = (Elf64_Ehdr *) e;
	
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
  ehdr->e_shoff = 0;                       // Section header offset XXX
  ehdr->e_flags = 0;                       // Flags
  ehdr->e_ehsize = sizeof(Elf64_Ehdr);     // Elf header size in bytes
  ehdr->e_phentsize = sizeof(Elf64_Phdr);  // Size of one entry in the program header table
  ehdr->e_phnum = 1;                       // Number of entries in the program header table
  ehdr->e_shentsize = 0;                   // Size of one entry in the section header table
  ehdr->e_shnum = 0;                       // Number of entries in the section header table
  ehdr->e_shstrndx = 0;                    // Section header name string index (should be: SHN_UNDEF)

  Elf64_Phdr *const phdr = (Elf64_Phdr *) ((uint8_t *) e + sizeof(Elf64_Ehdr));

  phdr->p_type = PT_LOAD;
  phdr->p_flags = PF_X | PF_R;
  phdr->p_offset = PAGE_SIZE;
  phdr->p_vaddr = start - pg_align_dist;
  phdr->p_paddr = 0;
  phdr->p_filesz = code_sz + pg_align_dist; 
  phdr->p_memsz = code_sz + pg_align_dist;
  phdr->p_align = 0x4;

  uint8_t *const data = (uint8_t *) e + PAGE_SIZE + pg_align_dist;
  memcpy(data, code, code_sz);

  *out = e;

  return sz;
}
