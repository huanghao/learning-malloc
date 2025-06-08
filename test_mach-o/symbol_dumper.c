#include <fcntl.h>
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

// 打印符号表
void print_symbol_table(struct mach_header_64 *header, void *file_data) {
  char *ptr = (char *)header + sizeof(struct mach_header_64);

  struct symtab_command *symtab = NULL;

  // 查找符号表load command
  for (int i = 0; i < header->ncmds; i++) {
    struct load_command *cmd = (struct load_command *)ptr;

    if (cmd->cmd == LC_SYMTAB) {
      symtab = (struct symtab_command *)cmd;
      break;
    }

    ptr += cmd->cmdsize;
  }

  if (!symtab) {
    printf("No symbol table found\n");
    return;
  }

  printf("=== Symbol Table ===\n");
  printf("Symbol table offset: %d\n", symtab->symoff);
  printf("Number of symbols: %d\n", symtab->nsyms);
  printf("String table offset: %d\n", symtab->stroff);
  printf("String table size: %d bytes\n\n", symtab->strsize);

  // 获取符号表和字符串表
  struct nlist_64 *symbols =
      (struct nlist_64 *)((char *)file_data + symtab->symoff);
  char *string_table = (char *)file_data + symtab->stroff;

  printf("Symbols:\n");
  printf("%-4s %-20s %-8s %-8s %-16s %s\n", "Idx", "Name", "Type", "Sect",
         "Value", "Description");
  printf("%-4s %-20s %-8s %-8s %-16s %s\n", "---", "----", "----", "----",
         "-----", "-----------");

  for (int i = 0; i < symtab->nsyms; i++) {
    struct nlist_64 *sym = &symbols[i];

    // 获取符号名称
    char *name = "";
    if (sym->n_un.n_strx > 0 && sym->n_un.n_strx < symtab->strsize) {
      name = string_table + sym->n_un.n_strx;
    }

    // 符号类型
    char type_char;
    uint8_t type = sym->n_type & N_TYPE;
    switch (type) {
      case N_UNDF:
        type_char = 'U';  // Undefined
        break;
      case N_ABS:
        type_char = 'A';  // Absolute
        break;
      case N_SECT:
        type_char = 'S';  // Section
        break;
      case N_PBUD:
        type_char = 'P';  // Prebound undefined
        break;
      case N_INDR:
        type_char = 'I';  // Indirect
        break;
      default:
        type_char = '?';
        break;
    }

    // 如果是外部符号，用小写字母表示
    if (sym->n_type & N_EXT) {
      if (type_char >= 'A' && type_char <= 'Z') {
        type_char = type_char - 'A' + 'a';
      }
    }

    // 符号描述
    const char *desc = "";
    if (sym->n_desc & N_WEAK_DEF) {
      desc = "weak def";
    } else if (sym->n_desc & N_WEAK_REF) {
      desc = "weak ref";
    }

    printf("%-4d %-20s %-8c %-8d 0x%-14llx %s\n", i, name, type_char,
           sym->n_sect, sym->n_value, desc);
  }
  printf("\n");
}

// 打印字符串表内容
void print_string_table(struct mach_header_64 *header, void *file_data) {
  char *ptr = (char *)header + sizeof(struct mach_header_64);

  struct symtab_command *symtab = NULL;

  // 查找符号表load command
  for (int i = 0; i < header->ncmds; i++) {
    struct load_command *cmd = (struct load_command *)ptr;

    if (cmd->cmd == LC_SYMTAB) {
      symtab = (struct symtab_command *)cmd;
      break;
    }

    ptr += cmd->cmdsize;
  }

  if (!symtab) {
    return;
  }

  printf("=== String Table Contents ===\n");
  char *string_table = (char *)file_data + symtab->stroff;

  printf("Offset  String\n");
  printf("------  ------\n");

  for (uint32_t i = 0; i < symtab->strsize;) {
    if (string_table[i] != '\0') {
      printf("%-6d  %s\n", i, &string_table[i]);
      i += strlen(&string_table[i]) + 1;
    } else {
      i++;
    }
  }
  printf("\n");
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
    printf("Example: %s simple_example\n", argv[0]);
    return 1;
  }

  const char *filename = argv[1];
  printf("Dumping symbols from: %s\n\n", filename);

  macho_file_t *file = map_file(filename);
  if (!file) {
    return 1;
  }

  struct mach_header_64 *header = (struct mach_header_64 *)file->data;

  // 检查magic number
  if (header->magic != MH_MAGIC_64 && header->magic != MH_MAGIC) {
    printf("Error: Not a valid Mach-O file or unsupported format\n");
    unmap_file(file);
    return 1;
  }

  print_symbol_table(header, file->data);
  print_string_table(header, file->data);

  unmap_file(file);
  return 0;
}