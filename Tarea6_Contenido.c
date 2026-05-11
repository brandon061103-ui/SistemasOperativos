#include <stdio.h> //Entrada y salida estándar: printf(), perror()
#include <stdint.h> //Tipos enteros de tamaño fijo: uint32_t, uint16_t, etc.
#include <stdlib.h> //Gestión de memoria dinámica: malloc(), free()
#include <unistd.h> //Llamadas al sistema: read(), lseek(), close()
#include <fcntl.h> //Control de archivos: open()
#include <math.h> //Funciones matemáticas

//Valor mágico que identifica sistemas de archivos EXT2 / EXT3 / EXT4.
//Siempre debe ser 0xEF53 si el superbloque es válido.
#define EXT4_SUPER_MAGIC 0xEF53

//El superbloque siempre inicia en el byte 1024 del dispositivo (independientemente del tamaño de bloque).
#define BASE_OFFSET 1024

//SUPERBLOCK EXT4 (estructura RAW)
//Contiene la metadata global del sistema de archivos
typedef struct {
    uint32_t s_inodes_count; //Total de inodos
    uint32_t s_blocks_count_lo; //Total de bloques
    uint32_t s_r_blocks_count_lo; //Bloques reservados para root
    uint32_t s_free_blocks_count_lo; //Bloques libres
    uint32_t s_free_inodes_count; //Inodos libres
    uint32_t s_first_data_block; //Primer bloque de datos
    uint32_t s_log_block_size; //log2(tamaño de bloque)
    uint32_t s_log_frag_size; //log2(tamaño de fragmento)
    uint32_t s_blocks_per_group; //Bloques por grupo
    uint32_t s_frags_per_group; //Fragmentos por grupo
    uint32_t s_inodes_per_group; //Inodos por grupo
    uint32_t s_mtime; //Último montaje
    uint32_t s_wtime; //Última escritura
    uint16_t s_mnt_count; //Conteo de montajes
    uint16_t s_max_mnt_count; //Máx. montajes antes de fsck (file system check)
    uint16_t s_magic; //Número mágico
    uint16_t s_state; //Estado del filesystem
    uint16_t s_errors; //Política de errores
    uint16_t s_minor_rev_level;  //Revisión menor
    uint32_t s_lastcheck; //Último fsck
    uint32_t s_checkinterval; //Intervalo entre fsck
    uint32_t s_creator_os; //Sistema Operativo creador
    uint32_t s_rev_level; //Nivel de revisión
    uint16_t s_def_resuid; //UID reservado (Identificador Único)
    uint16_t s_def_resgid; //GID reservado (Identificador de Grupo)

    uint32_t s_first_ino; //Primer inodo no reservado
    uint16_t s_inode_size; //Tamaño de inodo
    uint16_t s_block_group_nr; //Grupo actual (solo para copias)
    uint32_t s_feature_compat; //Features compatibles
    uint32_t s_feature_incompat; //Features incompatibles
    uint32_t s_feature_ro_compat; //Features solo lectura
    uint8_t  s_uuid[16]; //UUID del sistema (Universally Unique Identifier)
    char     s_volume_name[16]; //Nombre del volumen
    char     s_last_mounted[64]; //Último punto de montaje
} ext4_super_block;

//Group Descriptor
//Descriptor de grupo de bloques
//Describe las estructuras internas de cada grupo
typedef struct {
    uint32_t bg_block_bitmap; //Bloque del bitmap de bloques
    uint32_t bg_inode_bitmap; //Bloque del bitmap de inodos
    uint32_t bg_inode_table; //Inicio de la tabla de inodos
    uint16_t bg_free_blocks_count; //Bloques libres en el grupo
    uint16_t bg_free_inodes_count; //Inodos libres en el grupo
    uint16_t bg_used_dirs_count; //Directorios usados
    uint16_t bg_flags; //Flags del grupo
    uint32_t bg_reserved[3]; //Reservado
} ext4_group_desc;

int main() {
    // Abrimos el dispositivo en modo solo lectura
    int fd = open("/dev/sda", O_RDONLY);
    // Validación de error
    if (fd < 0) {
        perror("Error al abrir el dispositivo");
        return 1;
    }

    //Lectura del superbloque
    lseek(fd, BASE_OFFSET, SEEK_SET);

    ext4_super_block sb;
    read(fd, &sb, sizeof(sb));

    // Validación del sistema de archivos
    if (sb.s_magic != EXT4_SUPER_MAGIC) {
        printf("No es un sistema de archivos EXT válido\n");
        close(fd);
        return 1;
    }

    //Cálculo del tamaño real del bloque */
    uint32_t block_size = 1024 << sb.s_log_block_size;

    //Número total de grupos
    uint32_t total_groups = (sb.s_blocks_count_lo + sb.s_blocks_per_group - 1) / sb.s_blocks_per_group;

    //Imprimir el superbloque completo
    printf("SUPERBLOCK EXT4\n");
    printf("Magic number: 0x%X\n", sb.s_magic);
    printf("Volume name: %s\n", sb.s_volume_name);
    printf("UUID: ");
    for (int i = 0; i < 16; i++) printf("%02X", sb.s_uuid[i]);
    printf("\n");

    printf("Inodes count: %u\n", sb.s_inodes_count);
    printf("Blocks count: %u\n", sb.s_blocks_count_lo);
    printf("Reserved blocks: %u\n", sb.s_r_blocks_count_lo);
    printf("Free blocks: %u\n", sb.s_free_blocks_count_lo);
    printf("Free inodes: %u\n", sb.s_free_inodes_count);
    printf("First data block: %u\n", sb.s_first_data_block);
    printf("Block size: %u bytes\n", block_size);
    printf("Blocks per group: %u\n", sb.s_blocks_per_group);
    printf("Inodes per group: %u\n", sb.s_inodes_per_group);
    printf("Inode size: %u bytes\n", sb.s_inode_size);
    printf("Mount count: %u\n", sb.s_mnt_count);
    printf("Max mount count: %u\n", sb.s_max_mnt_count);
    printf("Last mount time: %u\n", sb.s_mtime);
    printf("Last write time: %u\n", sb.s_wtime);
    printf("Filesystem revision: %u\n", sb.s_rev_level);
    printf("Creator OS: %u\n", sb.s_creator_os);
    printf("Last mounted on: %s\n", sb.s_last_mounted);
    printf("Total block groups: %u\n\n", total_groups);

    //Group Descriptor Table (GDT)
    //Tabla de descriptores de grupo

    /* En EXT4 la GDT empieza en:
       - Bloque 2 si block_size = 1024
       - Bloque 1 si block_size > 1024 */
    uint32_t gdt_block = (block_size == 1024) ? 2 : 1;
    off_t gdt_offset = gdt_block * block_size;

    lseek(fd, gdt_offset, SEEK_SET);

    // Información por grupo
    for (uint32_t i = 0; i < total_groups; i++) {
        ext4_group_desc gd;
        read(fd, &gd, sizeof(gd));

        uint32_t first_block = i * sb.s_blocks_per_group;
        uint32_t last_block = first_block + sb.s_blocks_per_group - 1;
        if (last_block >= sb.s_blocks_count_lo)
            last_block = sb.s_blocks_count_lo - 1;

        printf("Grupo de bloque: %u\n", i);
        printf("Block range: %u - %u\n", first_block, last_block);
        printf("Block bitmap: %u\n", gd.bg_block_bitmap);
        printf("Inode bitmap: %u\n", gd.bg_inode_bitmap);
        printf("Inode table start: %u\n", gd.bg_inode_table);
        printf("Free blocks: %u\n", gd.bg_free_blocks_count);
        printf("Free inodes: %u\n", gd.bg_free_inodes_count);
        printf("Used directories: %u\n", gd.bg_used_dirs_count);
        printf("Flags: 0x%X\n", gd.bg_flags);
        printf("Reserved: %u %u %u\n\n", gd.bg_reserved[0], gd.bg_reserved[1], gd.bg_reserved[2]);
    }

    close(fd);
    return 0;
}
