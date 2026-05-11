#include <stdio.h> //Entrada y salida estándar: printf(), perror()
#include <stdint.h> //Tipos enteros de tamaño fijo: uint32_t, uint16_t, etc.
#include <stdlib.h> //Gestión de memoria dinámica: malloc(), free()
#include <unistd.h> //Permite la comunicación entre aplicaciones y el sistema operativo: read(), lseek(), close()
#include <fcntl.h> //Control de archivos: open()

//Valor mágico que identifica sistemas de archivos EXT2 / EXT3 / EXT4.
//Siempre debe ser 0xEF53 si el superbloque es válido.
#define EXT4_SUPER_MAGIC 0xEF53

// El superbloque EXT4 siempre empieza en el byte 1024 (independientemente del tamaño de bloque).
#define BASE_OFFSET 1024

// En EXT4, el inodo 2 corresponde al directorio raíz (/)
#define ROOT_INODE 2

//SUPERBLOCK EXT4 (estructura RAW)
//Contiene la metadata global del sistema de archivos. Toda la información básica para poder interpretar el disco.
typedef struct {
    uint32_t s_inodes_count; //Total de inodos en el sistema
    uint32_t s_blocks_count_lo; //Total de bloques
    uint32_t s_r_blocks_count_lo; //Bloques reservados
    uint32_t s_free_blocks_count_lo; //Bloques libres
    uint32_t s_free_inodes_count; //Inodos libres
    uint32_t s_first_data_block; //Primer bloque de datos válido
    uint32_t s_log_block_size; //Tamaño del bloque (log2)
    uint32_t s_log_frag_size; //Tamaño del fragmento
    uint32_t s_blocks_per_group; //Bloques por grupo
    uint32_t s_frags_per_group; //Fragmentos por grupo
    uint32_t s_inodes_per_group; //Inodos por grupo
    uint32_t s_mtime; //Último montaje
    uint32_t s_wtime; //Última escritura
    uint16_t s_mnt_count; //Conteo de montajes
    uint16_t s_max_mnt_count; //Máximo antes de fsck
    uint16_t s_magic; //Número mágico EXT4
    uint16_t s_state; //Estado del FS
    uint16_t s_errors; //Política de errores
    uint16_t s_minor_rev_level; //Revisión menor
    uint32_t s_lastcheck; //Última verificación
    uint32_t s_checkinterval; //Intervalo de chequeo
    uint32_t s_creator_os; //Sistema Operativo creador
    uint32_t s_rev_level; //Revisión
    uint16_t s_def_resuid; //UID reservado (Identificador Único)
    uint16_t s_def_resgid; // GID reservado (Identificador de Grupo)
    uint32_t s_first_ino; // Primer inodo usable
    uint16_t s_inode_size; // Tamaño de cada inodo
} ext4_super_block;

//Group Descriptor
//Descriptor de grupo de bloques
//Describe las estructuras internas de cada grupo
typedef struct {
    uint32_t bg_block_bitmap; //Bloque del bitmap de bloques
    uint32_t bg_inode_bitmap; //Bloque del bitmap de inodos
    uint32_t bg_inode_table; //Bloque donde inicia la tabla de inodos
    uint16_t bg_free_blocks_count; //Bloques libres en el grupo
    uint16_t bg_free_inodes_count; //Inodos libres
    uint16_t bg_used_dirs_count; //Directorios usados
    uint16_t bg_flags; //Flags del grupo
} ext4_group_desc;

//Inodo EXT4
//Representa un archivo o directorio.
typedef struct {
    uint16_t i_mode; //Tipo y permisos
    uint16_t i_uid; //UID propietario
    uint32_t i_size_lo; //Tamaño del archivo
    uint32_t i_atime; //Último acceso
    uint32_t i_ctime; //Creación
    uint32_t i_mtime; //Modificación
    uint32_t i_dtime; //Borrado
    uint16_t i_gid; //GID propietario
    uint16_t i_links_count; //Enlaces
    uint32_t i_blocks_lo; //Bloques usados
    uint32_t i_flags; //Flags
    uint32_t i_block[15]; //Punteros a bloques de datos
} ext4_inode;

//Entrada del directorio EXT4
typedef struct {
    uint32_t inode; //Inodo asociado
    uint16_t rec_len; //Tamaño del registro
    uint8_t name_len; // Longitud del nombre
    uint8_t file_type; // Tipo de archivo
    char name[]; // Nombre del archivo
} ext4_dir_entry;

int main() {

    //Abrimos el dispositivo en modo solo lectura que contiene el directorio raíz
    int fd = open("/dev/sdd", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    //Lectura del superbloque
    ext4_super_block sb;
    //El superbloque siempre inicia en el byte 1024, se mueve al byte donde empieza la estructura del superbloque
    lseek(fd, BASE_OFFSET, SEEK_SET);
    //Lectura de los bytes y se colocan en una estructura en memoria
    read(fd, &sb, sizeof(sb));

    //Validación de que sea EXT4
    if (sb.s_magic != EXT4_SUPER_MAGIC) {
        printf("No es un sistema EXT4 válido\n");
        close(fd);
        return 1;
    }

    // Calculo del tamaño real del bloque
    uint32_t block_size = 1024 << sb.s_log_block_size;
    printf("SUPERBLOQUE\n");
    printf("Block size: %u bytes\n", block_size);
    printf("Inodes: %u\n", sb.s_inodes_count);
    printf("Blocks: %u\n\n", sb.s_blocks_count_lo);

    //Group Descriptor Table (GDT)
    //Tabla de descriptores de grupo
    // El GDT se encuentra justo después del superbloque
    off_t gdt_offset = (sb.s_first_data_block + 1) * block_size;
    ext4_group_desc gd;
    //Se mueve al byte donde empieza la estructura del GDT
    lseek(fd, gdt_offset, SEEK_SET);
    //Lectura de los bytes y se colocan en una estructura en memoria
    read(fd, &gd, sizeof(gd));

    //Localizar el inodo 2 (/) (El directorio raíz)
    // Bloque donde inicia la tabla de inodos
    uint32_t inode_table_block = gd.bg_inode_table;
    // Offset real de la tabla de inodos
    off_t inode_table_offset = inode_table_block * block_size;
    // Offset exacto del inodo 2
    off_t root_inode_offset = inode_table_offset + (ROOT_INODE - 1) * sb.s_inode_size;
    ext4_inode root_inode;

    //Se mueve al byte donde se encuentra el directorio raíz
    lseek(fd, root_inode_offset, SEEK_SET);
    //Lectura de los bytes y se colocan en una estructura en memoria
    read(fd, &root_inode, sizeof(root_inode));

    printf("Directorio root (/)\n");
    //Lectura de los bloques de datos del directorio raíz
    //Buffer para leer bloques completos
    uint8_t *block = malloc(block_size);
    //EXT4 usa hasta 12 punteros directos
    for (int i = 0; i < 12; i++) {
        //Si el puntero es 0, no hay más datos
        if (root_inode.i_block[i] == 0)
            continue;
        //Nos movemos al bloque de datos
        lseek(fd, root_inode.i_block[i] * block_size, SEEK_SET);
        read(fd, block, block_size);
        uint32_t offset = 0;
        //Recorremos las entradas de directorio
        while (offset < block_size) {
            ext4_dir_entry *entry = (ext4_dir_entry *)(block + offset);
            //Entrada inválida
            if (entry->inode == 0)
                break;

            //Imprimir el nombre
            printf("%.*s\n", entry->name_len, entry->name);

            //Avanza a la siguiente entrada
            offset += entry->rec_len;
        }
    }
    free(block);
    close(fd);
    return 0;
//1. Lectura del superbloque para conocer el tamaño del bloque y la organización del sistema.
//2. Localizar en el Group Descriptor la tabla de inodos del grupo 0.
//3. Cálculo el offset del inodo (Directorio raíz).
//4. Los bloques de datos a los que apunta ese inodo contienen las entradas de directorio que permiten listar los archivos.
//Lo que replica el comportamiento de ls.
// Superbloque -> GDT -> Tabla de Inodos -> Inodo -> Bloques de datos -> Entrada de directorios
// Tamaño del bloque y organización del sistema -> Localizar la tabla de inodos (grupo 0)
}
