// Yuval Levy 205781966

#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <dirent.h>
#include <time.h>

#define NO_C_FILE "0"
#define COMPILATION_ERROR "10"
#define TIMEOUT "20"
#define WRONG "50"
#define SIMILAR "75"
#define EXCELLENT "100"

typedef struct InputDataStruct {
    int check;
    char studentPath[155];
    char myInputPath[155];
    char correctPath[155];
} InputDataStruct;

InputDataStruct createDataStruct(char *argv[]);

int createMemberFromInput(int fDirIn, char *member);

int checkFilesInStudent(char *nextDir, InputDataStruct myDataStruct, char *studentName);

int tryToCompileFile(char *filePath);

int resCsv(char *studentName, char *score);

int runExe(char *myInput);

int getCompareResult(char *solFile, char *correctFile);
/**
 * Function Name: createMemberFromInput
 * Function Input: int fDirIn, char *member
 * Function Output: int
 * Function Operation: Get a path to a file and read the content to create the struct member.
 */
int createMemberFromInput(int fDirIn, char *member) {
    char buffer[1];
    int firstChar, i = 0;
    //member = (char *) calloc(151, sizeof(char));
    while (true) {
        // Read 1 char each time.
        firstChar = read(fDirIn, buffer, 1);
        // Check if error happend while reading.
        if (firstChar < 0) {
            if(write(2, "Error in system call\n", 25)==-1){
                // Write failed.
            }
            return -1;
        }
        // Read until the end of the row.
        if (buffer[0] == '\n') {
            member[i] = '\0';
            break;
        }
        member[i] = buffer[0];
        i++;
    }
    return 0;
}
/**
 * Function Name: createDataStruct
 * Function Input: char *argv[]
 * Function Output: InputDataStruct
 * Function Operation: Get the path from argv and create our struct with all the paths we need.
 */
InputDataStruct createDataStruct(char *argv[]) {
    InputDataStruct myDataStruct;
    int fDirIn = open(argv[1], O_RDONLY);
    if (fDirIn < 0) {
        if(write(2, "Not a valid directory\n", 30)==-1){
            // Write failed.
        }
        // Will be a flag that error happened.
        myDataStruct.check = -1;
        close(fDirIn);
        return myDataStruct;
    }
    int check;
    // Create one of the paths of the struct.
    check = createMemberFromInput(fDirIn, myDataStruct.studentPath);
    if (check == -1) {
        // Will be a flag that error happened.
        myDataStruct.check = -1;
        close(fDirIn);
        return myDataStruct;
    }
    // Create one of the paths of the struct.
    check = createMemberFromInput(fDirIn, myDataStruct.myInputPath);
    if (check == -1) {
        // Will be a flag that error happened.
        myDataStruct.check = -1;
        close(fDirIn);
        return myDataStruct;
    }
    // Create one of the paths of the struct.
    check = createMemberFromInput(fDirIn, myDataStruct.correctPath);
    if (check == -1) {
        // Will be a flag that error happened.
        myDataStruct.check = -1;
        close(fDirIn);
        return myDataStruct;
    }
    close(fDirIn);
    return myDataStruct;
}
/**
 * Function Name: tryToCompileFile
 * Function Input: char *filePath
 * Function Output: int
 * Function Operation: Get a path to the file and try to compile it.
 */
int tryToCompileFile(char *filePath) {
    pid_t pid;
    int status;
    // Create a new process.
    if ((pid = fork()) < 0) {
        if(write(2, "Error in system call\n", 25)==-1){
            // Write failed.
        }
        return -1;
    } else {
        if (pid == 0) {
            // The child process.
            char *gcc[] = {"gcc", "-o", "myExecFile.out", filePath, NULL};
            if (execvp(gcc[0], gcc) == -1) {
                if(write(2, "Error in system call\n", 25)==-1){
                    // Write failed.
                }
                return -1;
            }
            //return 0;
        } else {
            // The parent process.
            wait(&status);
            if (WEXITSTATUS(status) == 0) {
                // Worked.
                return 0;
            }
        }
    }
    return 1;
}
/**
 * Function Name: resCsv
 * Function Input: char *studentName, char *score, int* firstLine
 * Function Output: int
 * Function Operation: Get the files name, a score, and if its the first line of the file, and prints inside the file
 * the relevant line according to the score.
 */
int resCsv(char *studentName, char *score) {
    int fileD;
    if ((fileD = open("results.csv", O_CREAT | O_APPEND | O_WRONLY, 0644)) < 0) {
        if(write(2, "Error in system call\n", 25)==-1){
            // Write failed.
        }
        return -1;
    }
    char myLine[160] = {0};
//    // Check if its the first line we are printing if the file so we will know if we need to add '\n' or not.
//    if ((!(*firstLine))!=0) {
//        strcat(myLine, "\n");
//        strcat(myLine, studentName);
//    } else {
//        // Change out flag so we will know we printed the first line in the file.
//        (*firstLine) = (*firstLine)-1;
//        strcat(myLine, studentName);
//    }
    strcat(myLine, studentName);
    strcat(myLine, ",");
    // According to the score we will print in the file the relevant line.
    if (strcmp(score, NO_C_FILE) == 0) { strcat(myLine, "NO_C_FILE,0"); }
    else if (strcmp(score, COMPILATION_ERROR) == 0) { strcat(myLine, "COMPILATION_ERROR,10"); }
    else if (strcmp(score, TIMEOUT) == 0) { strcat(myLine, "TIMEOUT,20"); }
    else if (strcmp(score, WRONG) == 0) { strcat(myLine, "WRONG,50"); }
    else if (strcmp(score, SIMILAR) == 0) { strcat(myLine, "SIMILAR,75"); }
    else { strcat(myLine, "EXCELLENT,100");}
    strcat(myLine, "\n");
    if (write(fileD, myLine, strlen(myLine)) == -1) {
        // write failed.
        return -1;
    }
    return 1;
}
/**
 * Function Name: runExe
 * Function Input: char *myInput
 * Function Output: int
 * Function Operation: Get out path and execute it, checking if we got a time out.
 */
int runExe(char *myInput) {
    pid_t pid;
    int status;
    time_t startT, endT;
    double dif = 0;
    int newFdIn, newFdOut;
    // Create a new process.
    time(&startT);
    if ((pid = fork()) < 0) {
        if(write(2, "Error in system call\n", 25)==-1){
            // Write failed.
        }
        return -1;
    } else {
        if (pid == 0) {
            if ((newFdIn = open(myInput, O_RDONLY, 0644)) < 0) {
                if(write(2, "Input/output File not exists\n", 25)==-1){
                    // Write failed.
                }
                return -1;
            }
            if ((newFdOut = open("results.txt", O_CREAT | O_TRUNC | O_RDWR, 0644)) < 0) {
                if(write(2, "Error in system call\n", 25)==-1){
                    // Write failed.
                }
                return -1;
            }
            dup2(newFdIn, 0);
            dup2(newFdOut, 1);
            close(newFdIn);
            close(newFdOut);
            char *run[] = {"./myExecFile.out", myInput, NULL};
            if (execvp(run[0], run) == -1) {
                if(write(2, "Error in system call\n", 25)==-1){
                    // Write failed.
                }
                return -1;
            }
        } else {
            // The parent process.
            while (waitpid(pid, &status, WNOHANG) != pid) {
                time(&endT);
                dif = difftime(endT, startT);
                // if the execution time lasted more than 5 seconds return 1
                if (dif > 3) {
                    kill(pid, SIGKILL);
                    wait(NULL);
                    return 1;
                }
            }
            return 0;
        }
    }
    return 1;
}
/**
 * Function Name: getCompareResult
 * Function Input: char *solFile, char *correctFile
 * Function Output: int
 * Function Operation: Compare the file we got and the correct result path using "Comp.out" file.
 */
int getCompareResult(char *solFile, char *correctFile) {
    pid_t pid;
    int status;
    // Create a new process.
    if ((pid = fork()) < 0) {
        if(write(2, "Error in system call\n", 25)==-1){
            // Write failed.
        }
        return -1;
    } else {
        if (pid == 0) {
            char *compare[] = {"./comp.out", solFile, correctFile, NULL};
            // The child process.
            if (execvp(compare[0], compare) == -1) {
                if(write(2, "Error in system call\n", 25)==-1){
                    // Write failed.
                }
                return -1;
            }
            return 1;
        } else {
            // The parent process.
            do {
                waitpid(pid, &status, WUNTRACED);
            } while ((!WIFEXITED(status) && !WIFSIGNALED(status)));
        }
    }
    return WEXITSTATUS(status);
}
/**
 * Function Name: checkFilesInStudent
 * Function Input: char *nextDir, InputDataStruct myDataStruct, char *studentName, int* firstLine
 * Function Output: int
 * Function Operation: Loop over the sub file we got, and look for C files.
 */
int checkFilesInStudent(char *nextDir, InputDataStruct myDataStruct, char *studentName) {
    struct dirent *directoryP;
    DIR *dir;
    // Try to open the path bc its not a directory for sure.
    if ((dir = opendir(nextDir)) != NULL) {
        // Loop over the sub directory and look for C files.
        while ((directoryP = readdir(dir)) != NULL) {
            if (directoryP->d_type == DT_REG && strcmp(directoryP->d_name, ".") != 0 &&
                strcmp(directoryP->d_name, "..") != 0) {
                // Check if the file is '.c' file.
                char *fileName = directoryP->d_name;
                if ((fileName[strlen(fileName) - 2] == '.') && (fileName[strlen(fileName) - 1] == 'c')) {
                    char fileInDir[155] = {0};
                    // Create the new path we want to check for the C file.
                    strcat(fileInDir, nextDir);
                    strcat(fileInDir, "/");
                    strcat(fileInDir, directoryP->d_name);
                    // Try to compile the file using this func.
                    int compileRes = tryToCompileFile(fileInDir);
                    // Check if error happened or compile error happened.
                    if (compileRes == -1 || compileRes == 1) {
                        closedir(dir);
                        // System call error.
                        if (compileRes == -1) {
                            return -1;
                        }
                        // Compile call error. Need to add it to the rcv with grade of 10.
                        resCsv(studentName, "10");
                        return 1;
                    }
                    // Comiple worked. Try to execute the file.
                    if (runExe(myDataStruct.myInputPath) == 1) {
                        // Time out happened.
                        resCsv(studentName, "20");
                        closedir(dir);
                        return 1;
                    }
                    // Compare the result of the execution that we got with the correct answer we have from
                    // in our struct.
                    int res = getCompareResult("results.txt", myDataStruct.correctPath);
                    if (res == 1) {
                        // The files are the same.
                        resCsv(studentName, "100");
                        closedir(dir);
                        return 1;
                    } else if (res == 3) {
                        // The files are half similar
                        resCsv(studentName, "75");
                        closedir(dir);
                        return 1;
                    } else if (res == 2) {
                        // The files are different.
                        resCsv(studentName, "50");
                        closedir(dir);
                        return 1;
                    }else{
                        // An error happened during comp.out.
                        return 0;
                    }
                }
            }
        }
        // No C file was found.
        resCsv(studentName, "0");
        closedir(dir);
        return 0;
    } else {
        if(write(2, "Not a valid directory\n", 30)==-1){
            // Write failed.
        }
        closedir(dir);
        return -1;
    }
}

int main(int argc, char *argv[]) {
    struct dirent *directoryP;
    DIR *dir;
    // Check that we get 1 args.
    if (argc != 2) {
        if(write(2, "Invalid input\n", 30)==-1){
            // Write failed.
        }
        return 0;
    }
    // Create a struct with all the paths we need from the input.
    InputDataStruct myDataStruct = createDataStruct(argv);
    if (myDataStruct.check == -1) {
        return 0;
    }
    // Try to open the path bc its not a directory for sure.
    if ((dir = opendir(myDataStruct.studentPath)) != NULL) {
        // Loop over and look for a directory and not files.
        while ((directoryP = readdir(dir)) != NULL) {
            if (directoryP->d_type == DT_DIR && strcmp(directoryP->d_name, ".") != 0 &&
                strcmp(directoryP->d_name, "..") != 0) {
                char nextDir[155] = {0};
                // Create our new path adding the directory name.
                strcat(nextDir, myDataStruct.studentPath);
                strcat(nextDir, "/");
                strcat(nextDir, directoryP->d_name);
                strcat(nextDir, "\0");
                checkFilesInStudent(nextDir, myDataStruct, directoryP->d_name);
            }
        }
        closedir(dir);
        // Delete all not relevant files we created during the program.
        remove("myExecFile.out");
        remove("results.txt");
    }else{
        if(write(2, "Not a valid directory\n", 30)==-1){
            // Write failed.
        }
        return -1;
    }
}