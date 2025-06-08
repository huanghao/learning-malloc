#include <fcntl.h>
#include <mach-o/fat.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct {
  void *data;
  size_t size;
} macho_file_t;

// 打印Mach-O header信息
void print_mach_header(struct mach_header_64 *header) {
  printf("=== Mach-O Header ===\n");
  printf("Magic: 0x%x ", header->magic);

  switch (header->magic) {
    case MH_MAGIC:
      printf("(32-bit)\n");
      break;
    case MH_MAGIC_64:
      printf("(64-bit)\n");
      break;
    case MH_CIGAM:
      printf("(32-bit, swapped)\n");
      break;
    case MH_CIGAM_64:
      printf("(64-bit, swapped)\n");
      break;
    default:
      printf("(unknown)\n");
      break;
  }

  printf("CPU Type: %d ", header->cputype);
  switch (header->cputype) {
    case CPU_TYPE_X86_64:
      printf("(x86_64)\n");
      break;
    case CPU_TYPE_ARM64:
      printf("(arm64)\n");
      break;
    default:
      printf("(other)\n");
      break;
  }

  printf("CPU Subtype: %d\n", header->cpusubtype);

  printf("File Type: %d ", header->filetype);
  switch (header->filetype) {
    case MH_OBJECT:
      printf("(Object file)\n");
      break;
    case MH_EXECUTE:
      printf("(Executable)\n");
      break;
    case MH_DYLIB:
      printf("(Dynamic library)\n");
      break;
    case MH_DYLINKER:
      printf("(Dynamic linker)\n");
      break;
    case MH_BUNDLE:
      printf("(Bundle)\n");
      break;
    default:
      printf("(other)\n");
      break;
  }

  printf("Number of load commands: %d\n", header->ncmds);
  printf("Size of load commands: %d bytes\n", header->sizeofcmds);
  printf("Flags: 0x%x\n", header->flags);
  printf("\n");
}

// 打印load command信息
void print_load_commands(struct mach_header_64 *header) {
  printf("=== Load Commands ===\n");

  char *ptr = (char *)header + sizeof(struct mach_header_64);

  for (int i = 0; i < header->ncmds; i++) {
    struct load_command *cmd = (struct load_command *)ptr;

    printf("Load Command #%d:\n", i + 1);
    printf("  Command: %d ", cmd->cmd);

    switch (cmd->cmd) {
      case LC_SEGMENT_64:
        printf("(SEGMENT_64)\n");
        struct segment_command_64 *seg = (struct segment_command_64 *)cmd;
        printf("  Segment name: %.16s\n", seg->segname);
        printf("  VM address: 0x%llx\n", seg->vmaddr);
        printf("  VM size: %llu bytes\n", seg->vmsize);
        printf("  File offset: %llu\n", seg->fileoff);
        printf("  File size: %llu bytes\n", seg->filesize);
        printf("  Number of sections: %d\n", seg->nsects);
        break;
      case LC_DYLD_INFO_ONLY:
        printf("(DYLD_INFO_ONLY)\n");
        break;
      case LC_SYMTAB:
        printf("(SYMTAB)\n");
        struct symtab_command *symtab = (struct symtab_command *)cmd;
        printf("  Symbol table offset: %d\n", symtab->symoff);
        printf("  Number of symbols: %d\n", symtab->nsyms);
        printf("  String table offset: %d\n", symtab->stroff);
        printf("  String table size: %d bytes\n", symtab->strsize);
        break;
      case LC_DYSYMTAB:
        printf("(DYSYMTAB)\n");
        break;
      case LC_LOAD_DYLINKER:
        printf("(LOAD_DYLINKER)\n");
        break;
      case LC_UUID:
        printf("(UUID)\n");
        break;
      case LC_VERSION_MIN_MACOSX:
        printf("(VERSION_MIN_MACOSX)\n");
        break;
      case LC_SOURCE_VERSION:
        printf("(SOURCE_VERSION)\n");
        break;
      case LC_MAIN:
        printf("(MAIN)\n");
        break;
      case LC_LOAD_DYLIB:
        printf("(LOAD_DYLIB)\n");
        break;
      case LC_FUNCTION_STARTS:
        printf("(FUNCTION_STARTS)\n");
        break;
      case LC_DATA_IN_CODE:
        printf("(DATA_IN_CODE)\n");
        break;
      default:
        printf("(other: 0x%x)\n", cmd->cmd);
        break;
    }

    printf("  Command size: %d bytes\n\n", cmd->cmdsize);
    ptr += cmd->cmdsize;
  }
}

// 打印段和节信息
void print_segments_and_sections(struct mach_header_64 *header) {
  printf("=== Segments and Sections ===\n");

  char *ptr = (char *)header + sizeof(struct mach_header_64);

  for (int i = 0; i < header->ncmds; i++) {
    struct load_command *cmd = (struct load_command *)ptr;

    if (cmd->cmd == LC_SEGMENT_64) {
      struct segment_command_64 *seg = (struct segment_command_64 *)cmd;

      printf("Segment: %.16s\n", seg->segname);
      printf("  VM Address: 0x%llx - 0x%llx\n", seg->vmaddr,
             seg->vmaddr + seg->vmsize);
      printf("  File Offset: %llu - %llu\n", seg->fileoff,
             seg->fileoff + seg->filesize);
      printf("  Permissions: ");
      if (seg->initprot & VM_PROT_READ)
        printf("r");
      else
        printf("-");
      if (seg->initprot & VM_PROT_WRITE)
        printf("w");
      else
        printf("-");
      if (seg->initprot & VM_PROT_EXECUTE)
        printf("x");
      else
        printf("-");
      printf("\n");

      // 打印该段中的节
      struct section_64 *sections = (struct section_64 *)(seg + 1);
      for (int j = 0; j < seg->nsects; j++) {
        struct section_64 *sect = &sections[j];
        printf("    Section: %.16s.%.16s\n", sect->segname, sect->sectname);
        printf("      Address: 0x%llx\n", sect->addr);
        printf("      Size: %llu bytes\n", sect->size);
        printf("      Offset: %d\n", sect->offset);
        printf("      Alignment: 2^%d\n", sect->align);
        printf("      Type: 0x%x\n", sect->flags & SECTION_TYPE);
        printf("      Attributes: 0x%x\n", sect->flags & SECTION_ATTRIBUTES);
      }
      printf("\n");
    }

    ptr += cmd->cmdsize;
  }
}

// 检查是否为Fat Binary
int is_fat_binary(void *data) {
  uint32_t magic = *(uint32_t *)data;
  return (magic == FAT_MAGIC || magic == FAT_CIGAM || magic == FAT_MAGIC_64 ||
          magic == FAT_CIGAM_64);
}

// 打印Fat Binary信息
void print_fat_header(void *data) {
  struct fat_header *fat = (struct fat_header *)data;
  printf("=== Fat Binary Header ===\n");
  printf("Magic: 0x%x\n", fat->magic);
  printf("Number of architectures: %d\n", fat->nfat_arch);
  printf("\n");

  struct fat_arch *archs = (struct fat_arch *)(fat + 1);
  for (int i = 0; i < fat->nfat_arch; i++) {
    printf("Architecture #%d:\n", i + 1);
    printf("  CPU Type: %d\n", archs[i].cputype);
    printf("  CPU Subtype: %d\n", archs[i].cpusubtype);
    printf("  Offset: %d\n", archs[i].offset);
    printf("  Size: %d bytes\n", archs[i].size);
    printf("  Alignment: 2^%d\n", archs[i].align);
    printf("\n");
  }
}

// 映射文件到内存
macho_file_t *map_file(const char *filename) {
  int fd = open(filename, O_RDONLY);
  if (fd < 0) {
    perror("open");
    return NULL;
  }

  struct stat st;
  if (fstat(fd, &st) < 0) {
    perror("fstat");
    close(fd);
    return NULL;
  }

  void *data = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data == MAP_FAILED) {
    perror("mmap");
    close(fd);
    return NULL;
  }

  close(fd);

  macho_file_t *file = malloc(sizeof(macho_file_t));
  file->data = data;
  file->size = st.st_size;

  return file;
}

// 释放文件映射
void unmap_file(macho_file_t *file) {
  if (file) {
    munmap(file->data, file->size);
    free(file);
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <mach-o file>\n", argv[0]);
    printf("Example: %s /bin/ls\n", argv[0]);
    return 1;
  }

  const char *filename = argv[1];
  printf("Analyzing Mach-O file: %s\n\n", filename);

  macho_file_t *file = map_file(filename);
  if (!file) {
    return 1;
  }

  // 检查是否为Fat Binary
  if (is_fat_binary(file->data)) {
    print_fat_header(file->data);

    // 对于Fat Binary，我们分析第一个架构
    struct fat_header *fat = (struct fat_header *)file->data;
    struct fat_arch *arch = (struct fat_arch *)(fat + 1);

    if (fat->nfat_arch > 0) {
      printf("Analyzing first architecture at offset %d:\n\n", arch->offset);
      struct mach_header_64 *header =
          (struct mach_header_64 *)((char *)file->data + arch->offset);

      print_mach_header(header);
      print_load_commands(header);
      print_segments_and_sections(header);
    }
  } else {
    // 单一架构的Mach-O文件
    struct mach_header_64 *header = (struct mach_header_64 *)file->data;

    // 检查magic number
    if (header->magic != MH_MAGIC_64 && header->magic != MH_MAGIC) {
      printf("Error: Not a valid Mach-O file\n");
      unmap_file(file);
      return 1;
    }

    print_mach_header(header);
    print_load_commands(header);
    print_segments_and_sections(header);
  }

  unmap_file(file);
  return 0;
}