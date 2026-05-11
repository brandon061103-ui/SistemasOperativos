#include <stdio.h> //Entrada y salida estándar, printf(), perror()
#include <stdint.h> //Tipos enteros con tamaño fijo, uint32_t, uint16_t, etc.
#include <stdlib.h> //Memoria dinámica, malloc(), free()
#include <unistd.h> //Proporciona acceso a la API del sistema operativo ,read(), lseek(), close()
#include <fcntl.h> //Realizar operaciones de control sobre descriptores de archivo, open()
#include <sys/types.h> //Define tipos de datos usados en programación a bajo nivel, off_t (Tamaños/desplazamientos de archivos)
#include <string.h> // Manejo de cadenas, memset(), string helpers

//Valor mágico de mi dispositivo que identifica un sistema de archivos EXT2/EXT3/EXT4
#define EXT4_SUPER_MAGIC 0xEF53

//Estructura de EXT4
//Estas estructura representa cómo están almacenados los datos dentro del disco. No son estructuras del kernel, son copias "RAW".

//Superblock EXT4
//El superblock contiene la metadata GLOBAL del sistema de archivos: tamaños, conteos, estado, etc.
//Siempre se encuentra a partir del byte 1024 del disco.

struct ext4_super_block {
    uint32_t s_inodes_count; //Número total de inodos del sistema
    uint32_t s_blocks_count_lo; //Número total de bloques (parte baja)
    uint32_t s_r_blocks_count_lo; //Bloques reservados para root
    uint32_t s_free_blocks_count_lo; //Bloques libres
    uint32_t s_free_inodes_count; //Inodos libres
    uint32_t s_first_data_block; //Primer bloque de datos
    uint32_t s_log_block_size; //Logaritmo base 2 del tamaño de bloque
    uint32_t s_log_cluster_size; //Tamaño del cluster (no se muestra aquí)
    uint32_t s_blocks_per_group; //Bloques por grupo
    uint32_t s_clusters_per_group; //Clusters por grupo
    uint32_t s_inodes_per_group; //Inodos por grupo
    uint32_t s_mtime; //Última vez montado
    uint32_t s_wtime; //Última vez escrito
    uint16_t s_mnt_count; //Número de montajes
    uint16_t s_max_mnt_count; //Máximo de montajes antes de fsck
    uint16_t s_magic; //Valor mágico (0xEF53)
    uint16_t s_state; //Estado del filesystem
    uint16_t s_errors; //Comportamiento ante errores
    uint16_t s_minor_rev_level; //Versión menor
    uint32_t s_lastcheck; // Última revisión
    uint32_t s_checkinterval; //Intervalo entre revisiones
    uint32_t s_creator_os; //Sistema operativo creador
    uint32_t s_rev_level; //Versión del filesystem
    uint16_t s_def_resuid;// UID reservado
    uint16_t s_def_resgid; //GID reservado
    uint32_t s_first_ino; //Primer inodo válido
    uint16_t s_inode_size; //Tamaño de cada inodo
};
   //Group Descriptor
   //Cada grupo de bloques tiene un descriptor que indica dónde están sus estructuras internas.
struct ext4_group_desc {
    uint32_t bg_block_bitmap; //Bloque donde inicia el bitmap de bloques
    uint32_t bg_inode_bitmap; //Bloque donde inicia el bitmap de inodos
    uint32_t bg_inode_table; //Bloque donde inicia la tabla de inodos
    uint16_t bg_free_blocks_count; // Cantidad de bloques libres en el grupo
    uint16_t bg_free_inodes_count; //Cantidad de inodos libres
    uint16_t bg_used_dirs_count; //Número de directorios usados
    uint16_t bg_flags; //Flags del grupo
};

int main() {
    //Se abre el dispositivo de bloque /dev/sda en modo lectura
    int fd = open("/dev/sda", O_RDONLY);
    // Validación de error
    if (fd < 0) {
        perror("open");
        return 1;
    }

    //Tamaño total del dispositivo
    //Mover al final del dispositivo
    off_t device_size = lseek(fd, 0, SEEK_END);

    //Regresamos al inicio
    lseek(fd, 0, SEEK_SET);

    //Lectura del superbloque
    struct ext4_super_block sb;
    // El superblock inicia en el byte 1024
    lseek(fd, 1024, SEEK_SET);
    // Lectura el superblock completo
    read(fd, &sb, sizeof(sb));

    // Validar que sea EXT4
    if (sb.s_magic != EXT4_SUPER_MAGIC) {
        printf("No es un sistema de archivos EXT4 válido\n");
        return 1;
    }

    //Cálculos realizados del superbloque
    uint32_t block_size = 1024 << sb.s_log_block_size; //Tamaño real del bloque
    uint32_t total_blocks = sb.s_blocks_count_lo; //Total de bloques
    uint32_t blocks_per_group = sb.s_blocks_per_group; //Bloques por grupo
    uint32_t inodes_per_group = sb.s_inodes_per_group; //Inodos por grupo
    uint32_t inode_size = sb.s_inode_size; //Tamaño de cada inodo
    uint32_t groups = (total_blocks + blocks_per_group - 1) / blocks_per_group; //Número total de grupos

    //Salida de la información como dumpe2fs
    printf("Filesystem volume name: <none>\n");
    printf("Filesystem magic number: 0x%X\n", sb.s_magic);
    printf("Filesystem revision #: %u\n", sb.s_rev_level);
    printf("Filesystem state: clean\n");
    printf("Errors behavior: Continue\n");
    printf("Filesystem OS type: Linux\n");
    printf("Inode count: %u\n", sb.s_inodes_count);
    printf("Block count: %u\n", total_blocks);
    printf("Reserved block count: %u\n", sb.s_r_blocks_count_lo);
    printf("Free blocks: %u\n", sb.s_free_blocks_count_lo);
    printf("Free inodes: %u\n", sb.s_free_inodes_count);
    printf("First block: %u\n", sb.s_first_data_block);
    printf("Block size: %u\n", block_size);
    printf("Fragment size: %u\n", block_size);
    printf("Blocks per group: %u\n", blocks_per_group);
    printf("Inodes per group: %u\n", inodes_per_group);
    printf("Inode size: %u\n\n", inode_size);

    //Group Descriptor Table (GDT)
    off_t gdt_offset = (sb.s_first_data_block + 1) * block_size; // La GDT se encuentra después del superblock
    struct ext4_group_desc *gdt = malloc(groups * sizeof(struct ext4_group_desc)); //Se reserva memoria para todos los descriptores
    lseek(fd, gdt_offset, SEEK_SET); // Se lee la tabla completa
    read(fd, gdt, groups * sizeof(struct ext4_group_desc));

    // Información por grupo
    for (uint32_t g = 0; g < groups; g++) {
        //Rango de bloques del grupo
        uint32_t first_block = g * blocks_per_group;
        uint32_t last_block = (g == groups - 1)? total_blocks - 1: first_block + blocks_per_group - 1;
        //Tamaño de la tabla de inodos en bloques
        uint32_t inode_table_blocks = (inodes_per_group * inode_size) / block_size;
        printf("Group %u: (Blocks %u-%u)\n", g, first_block, last_block);
        if (g == 0)
            printf(" Primary superblock at %u, Group descriptors at %u-%u\n", sb.s_first_data_block, (uint32_t)(gdt_offset / block_size), (uint32_t)(gdt_offset / block_size));

        printf(" Block bitmap at %u\n", gdt[g].bg_block_bitmap);
        printf(" Inode bitmap at %u\n", gdt[g].bg_inode_bitmap);
        printf(" Inode table at %u-%u\n", gdt[g].bg_inode_table, gdt[g].bg_inode_table + inode_table_blocks - 1);
        printf("  %u free blocks, %u free inodes, %u directories\n\n", gdt[g].bg_free_blocks_count, gdt[g].bg_free_inodes_count, gdt[g].bg_used_dirs_count);
    }

    // Recorrido del disco de principio a fin
    // Contador de bloques leídos
    uint64_t blocks_read = 0;
    //Buffer para leer cada bloque
    uint8_t *buffer = malloc(block_size);
    /* Lectura de todos los bloques del dispositivo */
    for (uint32_t b = 0; b < total_blocks; b++) {
        lseek(fd, (off_t)b * block_size, SEEK_SET);
        read(fd, buffer, block_size);
        blocks_read++;
    }
    //Resumen final del recorrido
    printf("Resultados del escaneo completo del dispositivo\n");
    printf("Total device size : %ld bytes\n", device_size);
    printf("Blocks scanned : %lu\n", blocks_read);
    printf("Scan coverage : 0 → %lu bytes\n", (unsigned long)blocks_read * block_size);
    printf("Device scan status : COMPLETADO\n");
    free(buffer); //Liberación de memoria
    free(gdt);

    //Se cierra el dispositivo después de analizarlo
    close(fd);

    return 0;
}
