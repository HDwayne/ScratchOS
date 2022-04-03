package application;

import java.io.*;

public class VirtualDisk {
    private Inode[] tabInode = new Inode[OsDefines.INODE_TABLE_SIZE];
    private User[] tabUser = new User[OsDefines.NB_USERS];
    private RandomAccessFile storage;


    public VirtualDisk(String Path) throws IOException {
        this.storage = new RandomAccessFile(Path + "d0", "rw");

        for (int i = 0; i < this.tabInode.length; i++) {
            this.tabInode[i] = new Inode();
        }
        for (int i = 0; i < this.tabUser.length; i++) {
            this.tabUser[i] = new User();
        }

        if(this.storage.length() == 0){
            System.out.println("Disque vide.");
        }
        else{
            this.readSuperBlock();
            this.readInodeTable();
            this.readUserTable();
        }
    }

    /**
     * This function saves the disk by writing the superblock, the inode table, and the user table to the disk
     */
    public void saveDisk(){
        try {
            this.writeSuperBlock();
            this.writeInodeTable();
            this.writeUserTable();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * Reads the inode table from the disk
     */
    public void readInodeTable() throws IOException {
        for (int i = 0; i < this.tabInode.length; i++) {
            this.tabInode[i].read(this, i);
        }
    }

    /**
     * Reads the user table from the file
     */
    public void readUserTable() throws IOException {
        for (int i = 0; i < this.tabUser.length; i++) {
            this.tabUser[i].read(this, i);
        }
    }

    /**
     * Write the inode table to the disk
     */
    public void writeInodeTable() throws IOException {
        for (int i = 0; i < this.tabInode.length; i++) {
            this.tabInode[i].write(this, i);
        }
    }

    /**
     * Write the super block to the disk
     */
    public void writeSuperBlock() throws IOException {
       SuperBlock.write(this);
    }

    /**
     * Write the user table to the file
     */
    public void writeUserTable() throws IOException {
        for (int i = 0; i < this.tabUser.length; i++) {
            this.tabUser[i].write(this, i);
        }
    }

    /**
     * Returns the storage file
     *
     * @return A RandomAccessFile object.
     */
    public RandomAccessFile getStorage(){
        return this.storage;
    }

    /**
     * Reads the superblock from the file
     */
    public void readSuperBlock() throws IOException{
        SuperBlock.read(this);
    }

    /**
     * Prints the superblock, the inode table, and the user table
     *
     * @return The string representation of the filesystem.
     */
    public String toString(){
        String str="";

        SuperBlock.print();
        System.out.println();
        str += "Inode Table :\n";
        StringBuilder strBuilder = new StringBuilder(str);
        for (int i = 0; i < this.tabInode.length; i++) {
            if(!this.tabInode[i].isFree()){
                strBuilder.append("inode ").append(i).append(" :\n").append(this.tabInode[i].toString()).append("\n");
            }
        }
        str = strBuilder.toString();

        str += "User Table :\n";
        StringBuilder strBuilder1 = new StringBuilder(str);
        for (int i = 0; i < this.tabUser.length; i++) {
            if(!this.tabUser[i].isFree()){
                strBuilder1.append("user ").append(i).append(" :\n").append(this.tabUser[i].toString()).append("\n");
            }
        }
        str = strBuilder1.toString();
        return str;
    }

    /**
     * Given an inodeId, return the corresponding inode
     *
     * @param inodeId the inode number of the inode to be returned
     * @return The inode object with the given inodeId.
     */
    public Inode getInode(int inodeId){
        return this.tabInode[inodeId];
    }

    /**
     * The function is used to defragment the disk. It starts by looking at the first inode. If it is not free and its
     * first byte is not the start of the users area, then it will move the file to the start of the users area. It will
     * then erase the file from the disk
     */
    public void defragmentation() throws IOException {
        int max = 0;
        int minmax = 0;
        if (!this.tabInode[0].isFree() && this.tabInode[0].getFirstByte() != OsDefines.USERS_START + (OsDefines.USER_SIZE * OsDefines.NB_USERS * OsDefines.BLOCK_SIZE)) {
            System.out.println("[Défragmentaion] Inode 0");
            max = this.tabInode[0].getFirstByte() + this.tabInode[0].getnBlock() * OsDefines.BLOCK_SIZE;
            File file = new File();
            file.read(this, this.tabInode[0]);
            file.eraseFromDisk(this, this.tabInode[0]);
            this.tabInode[0].setFirstByte(OsDefines.USERS_START + (OsDefines.USER_SIZE * OsDefines.NB_USERS * OsDefines.BLOCK_SIZE));
            file.write(this, this.tabInode[0]);
            minmax = this.tabInode[0].getFirstByte() + this.tabInode[0].getnBlock() * OsDefines.BLOCK_SIZE;
        }

        for (int i = 1; i < OsDefines.INODE_TABLE_SIZE; i++) {
            if (!this.tabInode[i].isFree() && this.tabInode[i].getFirstByte() != this.tabInode[i-1].getFirstByte() + this.tabInode[i-1].getnBlock()*4){
                max = this.tabInode[i].getFirstByte() + this.tabInode[i].getnBlock() * OsDefines.BLOCK_SIZE;
                System.out.println("[Défragmentaion] Inode " + i);
                File file = new File();
                file.read(this, this.tabInode[i]);
                file.eraseFromDisk(this, this.tabInode[i]);
                this.tabInode[i].setFirstByte(this.tabInode[i-1].getFirstByte() + this.tabInode[i-1].getnBlock()*4);
                file.write(this, this.tabInode[i]);
                minmax = this.tabInode[i].getFirstByte() + this.tabInode[i].getnBlock() * OsDefines.BLOCK_SIZE;
            }
        }
        SuperBlock.updateFirstByte(this);
        System.out.println("[Défragmentation] Espace sauvé : " + (max - minmax));
    }
}
