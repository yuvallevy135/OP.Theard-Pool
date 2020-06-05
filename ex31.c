// Yuval Levy 205781966

#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>

int getFileSize(int fileDirIn);

int seekStart(int fDin, int i);

int checkIfSimilar(int firstDirIn, int secondDirIn);

int checkIfHalfSimilar(int biggerDirIn, int smallerDirIn,int  sizeBiggerFile, int sizeSmallerFile);

/**
 * Function Name: checkIfSimilar
 * Function Input: int firstDirIn, int secondDirIn
 * Function Output: int
 * Function Operation: Gets 2 file descriptors and check if the files content is the same.
 */
int checkIfSimilar(int firstDirIn, int secondDirIn) {
    char firstDirBuffer[1] = {0}, secondDirBuffer[1] = {0};
    int shouldStop = 0, checkSeek1, checkSeek2;
    int firstDirChar, secondDirChar;
    while (!shouldStop){
        firstDirChar = read(firstDirIn, firstDirBuffer, 1);
        secondDirChar = read(secondDirIn, secondDirBuffer, 1);

        // Check if both of my files returned something.
        if(firstDirChar >0 && secondDirChar > 0){
            if(firstDirBuffer[0] != secondDirBuffer[0]){
                return 0;
            }
        }else{
            shouldStop=1;
        }
    }
    // Check if an error happened during reading from the files.
    if (firstDirChar == -1 || secondDirChar == -1){
        //write(2, "Error in system call\n", 21);
        return -1;
    }
    // Returning FD to the beginning.
    checkSeek1 = seekStart(firstDirIn,0);
    checkSeek2 = seekStart(secondDirIn,0);
    // Check if error happened during seek.
    if (checkSeek1 == -1 || checkSeek2 == -1){
        return -1;
    }
    return 1;
}
/**
 * Function Name: checkIfHalfSimilar
 * Function Input: int biggerDirIn, int smallerDirIn,int  sizeBiggerFile, int sizeSmallerFile
 * Function Output: int
 * Function Operation: Gets 2 file descriptors and check if the files content is half similar.
 * That means checking in any offset, if the the files are half the same.
 * If the sizes are not the same, then we check according to the smaller file.
 */
int checkIfHalfSimilar(int biggerDirIn, int smallerDirIn,int  sizeBiggerFile, int sizeSmallerFile){
    int similarGoal;
    // Checking the size of the files.
    if(sizeBiggerFile <= sizeSmallerFile){
        similarGoal = (sizeBiggerFile+1)/2;
    }else{
        similarGoal = (sizeSmallerFile+1)/2;
    }
    int seek1, seek2;
    //int firstDirChar, secondDirChar;
    int i,j,round;
    // Looping according to the bigger files.
    for (i=0;i<sizeBiggerFile;i++){
        int firstDirChar=0, secondDirChar=0;
        int counter=0;
        char firstDirBuffer[sizeBiggerFile-i], secondDirBuffer[sizeSmallerFile];
        // Get each time part of the bigger file.
        firstDirChar = read(biggerDirIn, firstDirBuffer, sizeBiggerFile-i);
        // Get each time the whole smaller file.
        secondDirChar = read(smallerDirIn, secondDirBuffer, sizeSmallerFile);
        // Check if both of my files returned something.
        if(firstDirChar >0 && secondDirChar > 0) {
            // Check if we are in a round which the bigger files became smaller then the smaller file.
            // so we wont miss any offset to check.
            if(sizeBiggerFile-i <= sizeSmallerFile){
                round = sizeBiggerFile-i;
            }else{
                round = sizeSmallerFile;
            }
            // check according to the bigger file in that round, and compare each char between the files.
            for(j=0;j<round;j++){
                if(firstDirBuffer[j] == secondDirBuffer[j]){
                    // Match has found.
                    counter++;
                }
            }
            // Check if we found enough matches.
            if(counter >= similarGoal){
                return 1;
            }
        }
        // Returning FD to the beginning.
        seek1 = seekStart(biggerDirIn,i+1);
        seek2 = seekStart(smallerDirIn,0);
        // Check if error happened during seek.
        if (seek1 == -1 || seek2 == -1){
            //write(2, "Error in system call\n", 21);
            return -1;
        }
    }
    seek1 = seekStart(biggerDirIn,0);
    return 2;
}


/**
 * Function Name: seekStart
 * Function Input: int fDin, int i
 * Function Output: int
 * Function Operation: Return both of the FD to the beginning.
 */
int seekStart(int fDin, int i) {
    int check;
    check = lseek(fDin, i, SEEK_SET);
    if (check == -1) {
        //write(2, "Error in system call\n", 21);
        return -1;
    }
    return 1;
}
/**
 * Function Name: getFileSize
 * Function Input: int fileDirIn
 * Function Output: int
 * Function Operation: check the size of the file according to the FD,
 */
int getFileSize(int fileDirIn) {
    int readFromFile;
    char readBuffer[1] = {0};
    bool shouldStop = true;
    int charCounter = 0;
    int begin;
    // Read char by char from file.
    while (shouldStop) {
        readFromFile = read(fileDirIn, readBuffer, 1);
        if (readFromFile == 0) {
            shouldStop=false;
        } else {
            charCounter++;
        }
    }
    // If error happened while trying to read from file.
    if (readFromFile == -1) {
        //write(2, "Error in system call\n", 21);
        return -1;
    }
    // Go back to the beginning of the file.
    begin = seekStart(fileDirIn,0);
    if (begin == -1) {
        return -1;
    }
    return charCounter;
}


int main(int argc, char *argv[]) {

    int firstDirIn, secondDirIn;
    int sizeFileOne, sizeFileSecond;
    int checkSimilar, checkIsHalfSimilar1,checkIsHalfSimilar2;

    // Check that we get 2 args.
    if (argc != 3) {
        //printf("Invalid input\n");
        return 0;
    }
    // Open both of the files.
    firstDirIn = open(argv[1], O_RDONLY);
    secondDirIn = open(argv[2], O_RDONLY);
    // Check that no error happened during opening the files.
    if (firstDirIn < 0) {
        close(firstDirIn);
    }
    if (secondDirIn < 0) {
        close(secondDirIn);
    }
    // Get the size of the 2 files.
    sizeFileOne = getFileSize(firstDirIn);
    sizeFileSecond = getFileSize(secondDirIn);

    // check if the files are the same or if error happened.
    if (sizeFileOne == sizeFileSecond) {
        checkSimilar = checkIfSimilar(firstDirIn, secondDirIn);
        // check if they are the same
        if (checkSimilar == 1) {
            //printf("same\n");
            close(firstDirIn);
            close(secondDirIn);
            return 1;
            // check if error happened.
        } else if (checkSimilar == -1) {
            close(firstDirIn);
            close(secondDirIn);
            return 0;
        }
    }
    // Check all possibilities of the offsets of the files. One time we "move" the bigger file, and the the smaller.
    checkIsHalfSimilar1 = checkIfHalfSimilar(firstDirIn, secondDirIn, sizeFileOne, sizeFileSecond);
    checkIsHalfSimilar2 = checkIfHalfSimilar(secondDirIn, firstDirIn,sizeFileSecond,sizeFileOne);

    // They are half similar.
    if(checkIsHalfSimilar1 == 1 || checkIsHalfSimilar2 == 1){
        close(firstDirIn);
        close(secondDirIn);
        //printf("half-similar\n");
        return 3;
        // They are different.
    }else if(checkIsHalfSimilar1 == 2 && checkIsHalfSimilar2 ==2){
        //printf("different\n");
        close(firstDirIn);
        close(secondDirIn);
        return 2;
        // There was an error.
    }else{
        close(firstDirIn);
        close(secondDirIn);
        return 0;
    }
}