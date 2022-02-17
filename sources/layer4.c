/**
* \file layer4.c
 * \brief Source code for layer4 of the ScratchOs: File System
 * \author HERZBERG Dwayne and BERLIN Florian
 * \version 0.1
 * \date 13 February 2022
*/
#include "../headers/layer4.h"



/**
 * @brief Check if file exists.
 * @param filename
 * @return int, index of the file
 */
int is_file_in_inode(char *filename){
    for (int i = 0; i < virtual_disk_sos->super_block.number_of_files; ++i) {
        if (strcmp(virtual_disk_sos->inodes[i].filename, (char *)filename) == 0) return i;
    }
    return INODE_TABLE_SIZE;
}

/**
 * \brief Create or edit a file in the disk storage and in the inodes table
 * @param filename
 * @param filedata
 */
void write_file(char * filename, file_t filedata, session_t user){
    int i_inode = is_file_in_inode(filename);
    if (i_inode != INODE_TABLE_SIZE){
        if (filedata.size <= virtual_disk_sos->inodes[i_inode].size){
            strcpy(virtual_disk_sos->inodes[i_inode].mtimestamp, timestamp());
            uint pos = virtual_disk_sos->inodes[i_inode].first_byte;
            block_t block;
            for (int i = 0; i < compute_nblock(filedata.size); i++) {
                block.data[0] = filedata.data[i*BLOCK_SIZE+0];
                block.data[1] = filedata.data[i*BLOCK_SIZE+1];
                block.data[2] = filedata.data[i*BLOCK_SIZE+2];
                block.data[3] = filedata.data[i*BLOCK_SIZE+3];
                write_block(block, (int)pos);
                pos+=BLOCK_SIZE;
            }
        }
        else{
            char *ctimestamp = malloc(sizeof(char)*TIMESTAMP_SIZE);
            strcpy(ctimestamp, virtual_disk_sos->inodes[i_inode].ctimestamp);
            delete_inode(i_inode);
            init_inode(filename, filedata.size, virtual_disk_sos->super_block.first_free_byte, ctimestamp, timestamp(), user);
            uint pos = virtual_disk_sos->inodes[is_file_in_inode(filename)].first_byte;
            block_t block;
            for (int i = 0; i < compute_nblock(filedata.size); i++) {
                block.data[0] = filedata.data[i*BLOCK_SIZE+0];
                block.data[1] = filedata.data[i*BLOCK_SIZE+1];
                block.data[2] = filedata.data[i*BLOCK_SIZE+2];
                block.data[3] = filedata.data[i*BLOCK_SIZE+3];
                write_block(block, (int)pos);
                pos+=BLOCK_SIZE;
            }
            free(ctimestamp);
        }
    }
    else{
        init_inode(filename, filedata.size, virtual_disk_sos->super_block.first_free_byte, timestamp(), timestamp(), user);
        block_t block;
        uint pos = virtual_disk_sos->inodes[is_file_in_inode(filename)].first_byte;
        for (int i = 0; i < compute_nblock(filedata.size); i++) {
            block.data[0] = filedata.data[i*BLOCK_SIZE+0];
            block.data[1] = filedata.data[i*BLOCK_SIZE+1];
            block.data[2] = filedata.data[i*BLOCK_SIZE+2];
            block.data[3] = filedata.data[i*BLOCK_SIZE+3];
            write_block(block, (int)pos);
            pos+=BLOCK_SIZE;
        }
    }
}


/**
 * \brief Read a file from the disk storage
 * @param filename
 * @param filedata
 * @return
 */
int read_file(char *filename, file_t *filedata) {
    int index_inode = is_file_in_inode(filename);
    if (index_inode == INODE_TABLE_SIZE) return 0;
    filedata->size = virtual_disk_sos->inodes[index_inode].size;
    uint pos = virtual_disk_sos->inodes[index_inode].first_byte;
    block_t block;
    for (int j = 0; j < compute_nblock(filedata->size); j++) {
        read_block(&block, (int)pos);
        filedata->data[j*BLOCK_SIZE+0] = block.data[0];
        filedata->data[j*BLOCK_SIZE+1] = block.data[1];
        filedata->data[j*BLOCK_SIZE+2] = block.data[2];
        filedata->data[j*BLOCK_SIZE+3] = block.data[3];
        pos+=BLOCK_SIZE;
    }
    //fprintf("Find : %s\n", filedata->data);
    return 1;
}

/**
 * \brief Delete a file from the inode table
 * @param filename
 * @return
 */
int delete_file(char *filename){
    int index_inode = is_file_in_inode(filename);
    if (index_inode == INODE_TABLE_SIZE) return 0;
    delete_inode(index_inode);
    return 1;
}

/**
 * \brief Write file from host to virtual disk
 * @param filename
 * @return
 */
int load_file_from_host(char *filename, session_t user){
    FILE * hostfile = fopen(filename, "r");
    if (hostfile == NULL){
        perror("fopen");
        return 0;
    }
    file_t sosfile;
    fseek(hostfile, 0, SEEK_END);
    sosfile.size = ftell(hostfile);
    fseek(hostfile, 0, SEEK_SET);

    fread(sosfile.data, sizeof(char), sosfile.size, hostfile);
    sosfile.data[sosfile.size] = '\0';
    fprintf(stdout, "Red : %s\n", sosfile.data);
    write_file(filename, sosfile, user);
    return 1;
}

// TODO ERROR
int store_file_to_host(char *filenamesos){
    int index_inode = is_file_in_inode(filenamesos);
    if (index_inode == INODE_TABLE_SIZE) return ERROR; // TODO: Error handling
    file_t file;
    read_file(filenamesos, &file);
    FILE * fd;
    
    fd = fopen(filenamesos, "w");
    if (fd == NULL) return 0;
    if (fseek(fd, 0, SEEK_SET) != 0) {
        fprintf(stderr, "Changement de position impossible\n");
        if (fclose(fd) == EOF) {
            fprintf( stderr, "Cannot close file\n" );
        }
        return ERROR;
    }
    int code = (int)fwrite(file.data, sizeof(uchar), file.size, fd);
    if (code != file.size){
        fprintf(stderr, "An error occurred while writing block\n");
        if (fclose(fd) == EOF) {
            fprintf( stderr, "Cannot close file\n" );
        }
        return ERROR;
    }
    if (fclose(fd) == EOF) {
        fprintf( stderr, "Cannot close file\n" );
        return ERROR;
    }
    return SUCCESS;
}
