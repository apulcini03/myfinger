#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <utmpx.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>


typedef struct {
    char login[32];
    char name[256];
    char directory[50];
    char shell[32];
    char tty[10];
    char idle[50];
    char weekDay[32];
    char hoursMinutes[32];
    char officePhone[32];
    char officeLocation[32];
} UserInfo;




// Function to convert timestamp to a human-readable format
void printTime(time_t time) {
    struct tm *tm_info = localtime(&time);
    char buffer[26];
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    printf("%s", buffer);
}

// Function to print details of a utmpx entry
void printUtmpxEntry(struct utmpx *ut) {
    printf("User: %s\n", ut->ut_user);
    printf("ID: %s\n", ut->ut_id);
    printf("Line: %s\n", ut->ut_line);
    printf("PID: %d\n", ut->ut_pid);
    printf("Type: %d\n", ut->ut_type);

    printf("Time: ");
    printTime(ut->ut_tv.tv_sec);
    printf("\n");

    printf("Host: %s\n", ut->ut_host);
    printf("\n");
}



// Function to get the idle time for the terminal
char* getIdleTime(double seconds) {


    // Get the current time
    struct timeval current_time;
    gettimeofday(&current_time, NULL);

    // Calculate the idle time in seconds
    double idle_time = difftime(current_time.tv_sec, seconds);


    static char buff[50];
    if (idle_time < 60) {
        buff[0] = '\0'; // Se minore di 1 minuto lascio vuoto
    } else if (idle_time < 3600) {
        int minutes = idle_time / 60;
        sprintf(buff, "%d", minutes);
    } else if (idle_time < 86400) {
        int hours = idle_time / 3600;
        int minutes = (idle_time - (hours * 3600)) / 60;
        sprintf(buff, "%d:%02d", hours, minutes);
    } else {
        int days = idle_time / (24 * 3600);
        sprintf(buff, "%dd", days);
    }


    return buff;
}


char* getCompleteIdle(double seconds) {


    // Get the current time
    struct timeval current_time;
    gettimeofday(&current_time, NULL);

    // Calculate the idle time in seconds
    double idle_time = difftime(current_time.tv_sec, seconds);


    static char buff[50];
    if (idle_time < 60) {
        buff[0] = '\0'; // Se minore di 1 minuto lascio vuoto
    } else if (idle_time < 3600) {
        int minutes = idle_time / 60;
        sprintf(buff, "idle 0:%02d", minutes);
    } else if (idle_time < 86400) {
        int hours = idle_time / 3600;
        int minutes = (idle_time - (hours * 3600)) / 60;
        sprintf(buff, "idle %d:%02d", hours, minutes);
    } else {
        int days = idle_time / (24 * 3600);
        int hours = (idle_time - (days * 24 * 3600)) / 3600;
        int minutes = (idle_time - (days * 24 * 3600) - (hours * 3600)) / 60;
        sprintf(buff, "idle %d days %d:%02d", days, hours, minutes);
    }


    return buff;
}


char* getFormattedWeekDay(time_t aTime)
{

    static char time_buf[32];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&aTime);

    double difference = difftime(now, aTime) / (60 * 60 * 24); // Calcola la differenza in giorni

    if (difference <= 6) {
        strftime(time_buf, sizeof(time_buf), "%a", tm_info); // Ritorna il giorno della settimana se è più recente di 5 giorni
    } else {
        strftime(time_buf, sizeof(time_buf), "%b %d", tm_info); // Ritorna la data, altrimenti
    }

    return time_buf;

    /*
    static char time_buf[32];
    struct tm *tm_info = localtime(&aTime);
    strftime(time_buf, sizeof(time_buf), "%a", tm_info);

    return time_buf;
     */
}

char* getFormattedHoursMinutes(time_t aTime)
{
    static char time_buf[32];
    struct tm *tm_info = localtime(&aTime);
    strftime(time_buf, sizeof(time_buf), "%H:%M", tm_info);
    //printf("%s ", time_buf);

    return time_buf;
}


/*
// Function to get the idle time for the terminal
char* getIdleTime(const char *tty_name) {
    struct stat st;
    char path[256];
    snprintf(path, sizeof(path), "/dev/%s", tty_name);

    // Get the terminal's last access time
    if (stat(path, &st) != 0) {
        perror("stat");
        return "err";
    }

    // Get the current time
    struct timeval current_time;
    gettimeofday(&current_time, NULL);

    // Calculate the idle time in seconds
    double idle_time = difftime(current_time.tv_sec, st.st_atime);
    printf("Seconds: %f\n", idle_time);


    static char buff[50];
    if (idle_time < 60) {
        buff[0] = '\0'; // Se minore di 1 minuto lascio vuoto
    } else if (idle_time < 3600) {
        double minutes = idle_time / 60;
        sprintf(buff, "%f", minutes);
    } else if (idle_time < 86400) {
        double hours = idle_time / 3600;
        double minutes = (idle_time - (hours * 3600)) / 60;
        sprintf(buff, "%f:%f", hours, minutes);
    } else {
        double days = idle_time / (24 * 3600);
        sprintf(buff, "%fd", days);
    }

    printf("idletim=%s  \n", buff);



    return buff;
}
*/


UserInfo parseGecos(const char *gecos, UserInfo userInfo) {
    char buffer[256];
    strncpy(buffer, gecos, sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0'; // Serve a garantire che la stringa 'buffer' sia correttamente terminata con un carattere null

    char *token;
    token = strtok(buffer, ",");
    if (token != NULL) strncpy(userInfo.name, token, sizeof(userInfo.name));

    token = strtok(NULL, ",");
    if (token != NULL) {
        strncpy(userInfo.officeLocation, token, sizeof(userInfo.officeLocation));
    } else {
        strncpy(userInfo.officeLocation, "", sizeof(userInfo.officeLocation));
    }

    token = strtok(NULL, ",");
    if (token != NULL) {
        strncpy(userInfo.officePhone, token, sizeof(userInfo.officePhone));
    } else {
        strncpy(userInfo.officePhone, "", sizeof(userInfo.officePhone));
    }

    //printf("NOME: %s, OFFICE: %s, PHONE: %s\n", userInfo.name, userInfo.officeLocation, userInfo.officePhone);

    return userInfo;
}


int getLastLoginUnloggedUser(const char *username, time_t *last_login_time, char *last_login_tty, size_t tty_size) {
    struct utmpx ut;
    FILE *fp = fopen("var/run/utmpx", "rb");
    if (!fp) {
        return -1;
    }

    int found = 0;
    while (fread(&ut, sizeof(ut), 1, fp) == 1) {
        if (ut.ut_type == USER_PROCESS && strcmp(ut.ut_user, username) == 0) {
            *last_login_time = ut.ut_tv.tv_sec;
            strncpy(last_login_tty, ut.ut_line, tty_size - 1);
            last_login_tty[tty_size - 1] = '\0'; // Assicurati che sia null-terminated
            found = 1;
        }
    }

    fclose(fp);
    return found;
}


UserInfo getUserInfo(struct utmpx *ut, struct passwd *pwd){
    UserInfo userInfo;

    // Riempie la struttura UserInfo
    strncpy(userInfo.login, ut->ut_user, sizeof(userInfo.login));
    strncpy(userInfo.directory, pwd->pw_dir, sizeof(userInfo.directory));
    strncpy(userInfo.shell, pwd->pw_shell, sizeof(userInfo.shell));

    userInfo = parseGecos(pwd->pw_gecos, userInfo);

    //printf("NOME: %s, OFFICE: %s, PHONE: %s\n", userInfo.name, userInfo.officeLocation, userInfo.officePhone);

    char *weekDay = getFormattedWeekDay(ut->ut_tv.tv_sec);
    strncpy(userInfo.weekDay, weekDay, sizeof(userInfo.weekDay));

    char *hoursMinutes = getFormattedHoursMinutes(ut->ut_tv.tv_sec);
    strncpy(userInfo.hoursMinutes, hoursMinutes, sizeof(userInfo.hoursMinutes));

    strncpy(userInfo.tty, ut->ut_line, sizeof(userInfo.tty));

    //CALCOLO IDLE TIME
    if (strcmp(userInfo.tty, "console") == 0) {
        //CALCOLO IDEL TIME (per CONSOLE é diverso rispetto agli altri device)
        char *idleTime = getIdleTime(ut->ut_tv.tv_sec);
        strncpy(userInfo.idle, idleTime, sizeof(userInfo.idle));
        //strncpy(userInfo.tty, "*console", sizeof(userInfo.tty));
    } else {
        struct stat f_info;
        char path[50] = "/dev/";
        strcat(path, userInfo.tty);
        const char *file_name = path;

        //LEGGO I DATI DEL DEVICE
        if (stat(file_name, &f_info) != 0) {
            perror("stat error");
        }

        //CALCOLO IDLE TIME
        char *idleTime = getIdleTime(f_info.st_mtime);
        strncpy(userInfo.idle, idleTime, sizeof(userInfo.idle));

        //COME FA IL COMANDO FINGER PRENDO SOLO ULTIMI CARATTERI DEL DEVICE
        //snprintf(userInfo.tty, sizeof(userInfo.tty), " %s", &ut->ut_line[3]); //prendo solo gli ultimi caratteri ELIMINANDO I PRIMI 3
    }

    return userInfo;
}


// Funzione per verificare se un utente è già presente nell'array
int is_user_present(char *users[], int user_count, char *username) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i], username) == 0) {
            return 1;
        }
    }
    return 0;
}


/* PER (message off)
#include <stdio.h>
#include <stdlib.h>
#include <utmp.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

int main() {
    struct utmp *ut;
    char *tty;
    int fd;
    struct stat statbuf;

    // Ottieni il nome del terminale
    tty = ttyname(STDIN_FILENO);
    if (tty == NULL) {
        perror("ttyname");
        exit(EXIT_FAILURE);
    }

    // Ottieni informazioni dal file utmp
    setutent();  // Apre il file utmp
    while ((ut = getutent()) != NULL) {
        if (ut->ut_type == USER_PROCESS && strcmp(ut->ut_line, tty + strlen("/dev/")) == 0) {
            // Controlla il permesso di scrittura sul terminale
            fd = open(tty, O_WRONLY);
            if (fd == -1) {
                perror("open");
                endutent();
                exit(EXIT_FAILURE);
            }

            if (ioctl(fd, TIOCSTAT, &statbuf) == -1) {
                perror("ioctl");
                close(fd);
                endutent();
                exit(EXIT_FAILURE);
            }

            if (statbuf.st_mode & S_IWUSR) {
                printf("Il permesso di scrittura è consentito (messages on).\n");
            } else {
                printf("Il permesso di scrittura è negato (messages off).\n");
            }

            close(fd);
            break;
        }
    }

    endutent();  // Chiude il file utmp
    return 0;
}
*/



/*
char* getLongLastLoginTime(struct utmpx *ut){
    // Last login in formato Fri May 31 23:14 (CEST)
    time_t lastLoginTime = 0;

    if (ut->ut_tv.tv_sec > lastLoginTime) {
        lastLoginTime = ut->ut_tv.tv_sec;
    }

    char last_login[64];
    struct tm *tm_info = localtime(&lastLoginTime);
    strftime(last_login, sizeof(last_login), "%a %b %d %H:%M (%Z)", tm_info);

    return last_login;
}
*/


// Funzione per verificare l'esistenza di un file
int checkIfFileExists(const char *path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

// Funzione per verificare se l'utente ha nuovi messaggi di posta
void checkMailStatus(const char *username) {
    char mail_path[256];
    snprintf(mail_path, sizeof(mail_path), "/var/mail/%s", username);

    //printf("\nCHECK MAIL\n\n");

    if (checkIfFileExists(mail_path)) {

        int fd = open(mail_path, O_RDONLY); // Apro il file della casella di posta in modalità di sola lettura

        char buffer[1024];
        ssize_t bytesRead;
        int foundDate = 0;

        while ((bytesRead = read(fd, buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0'; // Aggiungo il terminatore di stringa

            // Trova la prima riga che inizia con "From "
            char *line = strtok(buffer, "\n");
            while (line != NULL) {
                if (strncmp(line, "From ", 5) == 0) {
                    // Trova la data che segue immediatamente dopo "From [email] "
                    char *date_start = strrchr(line, ' ');
                    if (date_start != NULL && strlen(date_start + 1) == 24) {
                        printf("New Mail received %s\n", date_start + 1); // Stampa gli ultimi 24 caratteri (la data)
                        foundDate = 1;
                        break;
                    }
                }
                line = strtok(NULL, "\n");
            }

            if (foundDate) {
                break;
            }
        }

        if (bytesRead == -1) {
            perror("Errore nella lettura del file");
        } else if (!foundDate) {
            printf("No new mail.\n");
        }


        //Se non trovo la stringa "Date:", allora la casella di posta è vuota
        close(fd); // Chiudo il file della casella di posta
        printf("No mail 1.\n");

    } else {
        printf("No Mail 2.\n");
    }
}

// Funzione per verificare la presenza dei file .plan nella home directory dell'utente
void checkPlanStatus(const char *home_directory) {
    char plan_path[256];
    snprintf(plan_path, sizeof(plan_path), "%s/.plan", home_directory);

    if (checkIfFileExists(plan_path)) {
        printf("Plan:\n");
        // Apro il file .plan e stampo il contenuto
        FILE *file = fopen(plan_path, "r");
        if (file != NULL) {
            char line[256];
            while (fgets(line, sizeof(line), file)) {
                printf("%s", line);
            }
            fclose(file);
        } else {
            perror("fopen");
        }
    } else {
        printf("No Plan.\n");
    }
}


int main(int argc, char *argv[]) {

    struct utmpx *ut;
    // Open the utmpx file
    setutxent();



    printf("----------------------------------------------------------------------------------------------------\n");
    printf("Numero argomenti: %d\n", argc);

    printf("Stampo il contenuto di argv\n");
    for (int i = 0; i < argc; i++) {
        printf("Argomento %d: %s\n", i, argv[i]);
    }

    if (argc == 1) {
        //printf("Finger caso base: devo stampare le info su tutti gli utenti (o solo loggati?)\n");

        // Read each entry in the utmpx file


        printf("%-17s%-17s%-9s%-6s%-7s%-7s%-8s%s\n", "Login", "Name", "TTY", "Idle", "Login", "Time", "Office", "Phone");

        while ((ut = getutxent()) != NULL) {
            if (ut->ut_type == USER_PROCESS) {
                //printUtmpxEntry(ut);
                struct passwd *pwd = getpwnam(ut->ut_user);
                if (pwd != NULL) {
                    UserInfo userInfo = getUserInfo(ut, pwd);

                    if (strcmp(userInfo.tty, "console") == 0) {
                        strncpy(userInfo.tty, "*console", sizeof(userInfo.tty));
                    } else {
                        snprintf(userInfo.tty, sizeof(userInfo.tty), " %s", &ut->ut_line[3]); //Cosa faccio qui?
                    }

                    printf("%-17s%-16s%-8s%6s  %-7s%-7s%-8s%s\n", userInfo.login, userInfo.name, userInfo.tty, userInfo.idle, userInfo.weekDay, userInfo.hoursMinutes, userInfo.officeLocation, userInfo.officePhone);
                }
            }
        }

        /*
        while ((ut = getutxent()) != NULL) {
            if (ut->ut_type == USER_PROCESS) {
                struct passwd *pwd = getpwnam(ut->ut_user);
                if (pwd != NULL) {
                    UserInfo userInfo = getUserInfo(ut, pwd);

                    if (strcmp(userInfo.tty, "console") == 0) {
                        strncpy(userInfo.tty, "*console", sizeof(userInfo.tty));
                    } else {
                        snprintf(userInfo.tty, sizeof(userInfo.tty), " %s", &ut->ut_line[3]); //Cosa faccio qui?
                    }

                    printf("%-17s%-16s%-8s%6s  %-7s%-7s%-8s%s\n", userInfo.login, userInfo.name, userInfo.tty, userInfo.idle, userInfo.weekDay, userInfo.hoursMinutes, userInfo.officeLocation, userInfo.officePhone);                }
            }
        }
        */


    } else {
        if (argv[1][0] == '-') {
            printf("L'argomento %s inizia con il carattere '-'\n", argv[1]);
            if (argv[1][2] == '\0') {
                if (argc <= 2) {
                    switch (argv[1][1]) {
                        case 'l':
                            printf("Caso '-l', dovrò implementare l'utilizzo del comando finger con l'opzione richiesta\n\n");

                            char *users[100];
                            int user_count = 0;

                            // Leggo le voci dal file utmp
                            while ((ut = getutxent()) != NULL) {
                                if (ut->ut_type == USER_PROCESS) {
                                    // Copio il nome dell'utente nell'array se non è già presente
                                    if (!is_user_present(users, user_count, ut->ut_user)) {
                                        users[user_count] = malloc(32 * sizeof(char));
                                        if (users[user_count] != NULL) {
                                            strncpy(users[user_count], ut->ut_user, 32 - 1);
                                            users[user_count][32 - 1] = '\0'; // Mi assicuro che la stringa sia null-terminated
                                            user_count++;
                                        }
                                    }
                                }
                            }

                            for (int i = 0; i < user_count; i++) {

                                setutxent();

                                time_t lastLoginTime = 0;
                                struct passwd *pwd = NULL;
                                int user_info_printed = 0;
                                char last_login[64];

                                while ((ut = getutxent()) != NULL) {
                                    if (ut->ut_type == USER_PROCESS && strcmp(ut->ut_user, users[i]) == 0) {

                                        struct passwd *current_pwd = getpwnam(ut->ut_user);
                                        if (current_pwd != NULL) {
                                            if (pwd == NULL) {
                                                pwd = current_pwd;
                                            }

                                            if (ut->ut_tv.tv_sec > lastLoginTime) {
                                                lastLoginTime = ut->ut_tv.tv_sec;
                                            }


                                            struct tm *tm_info = localtime(&lastLoginTime);
                                            strftime(last_login, sizeof(last_login), "%a %b %d %H:%M (%Z)", tm_info);
                                        }
                                    }
                                }

                                setutxent();  // Reset to beginning of utmpx entries

                                while ((ut = getutxent()) != NULL) {
                                    if (ut->ut_type == USER_PROCESS) {
                                        struct passwd *current_pwd = getpwnam(ut->ut_user);
                                        if (current_pwd != NULL && pwd != NULL && strcmp(current_pwd->pw_name, pwd->pw_name) == 0) {
                                            if (!user_info_printed) {
                                                UserInfo userInfo = getUserInfo(ut, pwd);
                                                printf("Login: %-33sName: %s\n", userInfo.login, userInfo.name);
                                                printf("Directory: %-29sShell: %s\n", userInfo.directory, userInfo.shell);
                                                user_info_printed = 1;
                                            }

                                            UserInfo userInfo = getUserInfo(ut, current_pwd);
                                            char *idleTime;
                                            if (strcmp(userInfo.tty, "console") == 0) {
                                                idleTime = getCompleteIdle(ut->ut_tv.tv_sec);
                                            } else {
                                                struct stat f_info;
                                                char path[50] = "/dev/";
                                                strcat(path, userInfo.tty);

                                                if (stat(path, &f_info) != 0) {
                                                    perror("stat error");
                                                    continue;
                                                }

                                                idleTime = getCompleteIdle(f_info.st_mtime);
                                            }
                                            strncpy(userInfo.idle, idleTime, sizeof(userInfo.idle));
                                            printf("On since %s on %s,       %s ?(messages off)?\n", last_login, userInfo.tty, userInfo.idle);

                                            //TODO devo stampare anche l'altro utente!!!
                                        }
                                    }
                                }

                                if (pwd != NULL) {
                                    checkMailStatus(pwd->pw_name);
                                    checkPlanStatus(pwd->pw_dir);
                                }
                                printf("\n");

                            }

                            break;

                        case 's':
                            printf("Caso '-s', dovrò implementare l'utilizzo del comando finger con l'opzione richiesta\n\n");

                            printf("%-17s%-17s%-9s%-6s%-7s%-7s%-8s%s\n", "Login", "Name", "TTY", "Idle", "Login", "Time", "Office", "Phone");

                            while ((ut = getutxent()) != NULL) {
                                if (ut->ut_type == USER_PROCESS) {
                                    struct passwd *pwd = getpwnam(ut->ut_user);
                                    if (pwd != NULL) {
                                        UserInfo userInfo = getUserInfo(ut, pwd);

                                        if (strcmp(userInfo.tty, "console") == 0) {
                                            strncpy(userInfo.tty, "*console", sizeof(userInfo.tty));
                                        } else {
                                            snprintf(userInfo.tty, sizeof(userInfo.tty), " %s", &ut->ut_line[3]); //Cosa faccio qui?
                                        }

                                        printf("%-17s%-16s%-8s%6s  %-7s%-7s%-8s%s\n", userInfo.login, userInfo.name, userInfo.tty, userInfo.idle, userInfo.weekDay, userInfo.hoursMinutes, userInfo.officeLocation, userInfo.officePhone);                }
                                }
                            }

                            break;

                        case 'm':
                            printf("Caso '-m', dovrò implementare l'utilizzo del comando finger con l'opzione richiesta\n\n");

                            printf("%-17s%-17s%-9s%-6s%-7s%-7s%-8s%s\n", "Login", "Name", "TTY", "Idle", "Login", "Time", "Office", "Phone");

                            while ((ut = getutxent()) != NULL) {
                                if (ut->ut_type == USER_PROCESS) {
                                    struct passwd *pwd = getpwnam(ut->ut_user);
                                    if (pwd != NULL) {
                                        UserInfo userInfo = getUserInfo(ut, pwd);

                                        if (strcmp(userInfo.tty, "console") == 0) {
                                            strncpy(userInfo.tty, "*console", sizeof(userInfo.tty));
                                        } else {
                                            snprintf(userInfo.tty, sizeof(userInfo.tty), " %s", &ut->ut_line[3]); //Cosa faccio qui?
                                        }

                                        printf("%-17s%-16s%-8s%6s  %-7s%-7s%-8s%s\n", userInfo.login, userInfo.name, userInfo.tty, userInfo.idle, userInfo.weekDay, userInfo.hoursMinutes, userInfo.officeLocation, userInfo.officePhone);
                                    }
                                }
                            }

                            break;

                        case 'p':
                            printf("Caso '-p', dovrò implementare l'utilizzo del comando finger con l'opzione richiesta\n\n");

                            printf("%-17s%-17s%-9s%-6s%-7s%-7s%-8s%s\n", "Login", "Name", "TTY", "Idle", "Login", "Time", "Office", "Phone");

                            while ((ut = getutxent()) != NULL) {
                                if (ut->ut_type == USER_PROCESS) {
                                    struct passwd *pwd = getpwnam(ut->ut_user);
                                    if (pwd != NULL) {
                                        UserInfo userInfo = getUserInfo(ut, pwd);

                                        if (strcmp(userInfo.tty, "console") == 0) {
                                            strncpy(userInfo.tty, "*console", sizeof(userInfo.tty));
                                        } else {
                                            snprintf(userInfo.tty, sizeof(userInfo.tty), " %s", &ut->ut_line[3]); //Cosa faccio qui?
                                        }

                                        printf("%-17s%-16s%-8s%6s  %-7s%-7s%-8s%s\n", userInfo.login, userInfo.name, userInfo.tty, userInfo.idle, userInfo.weekDay, userInfo.hoursMinutes, "", "");                }
                                }
                            }

                            break;

                        default:
                            printf("L'argomento %s non rientra nel caso di '-l', '-s', '-m' o '-p'.\n", argv[1]);
                            break;

                    }
                } else {
                    printf("L'argomento %s rientra nel caso di '-l', '-s', '-m' o '-p'? Ha un utente dopo -lmps.\n", argv[1]);

                    switch (argv[1][1]) {
                        case 'l':
                            for (int i = 2; i < argc; i++) {

                                char *username = argv[i];
                                struct passwd *pwd = getpwnam(username);

                                if (pwd == NULL) {
                                    fprintf(stderr, "\nUtente %s non trovato!\n\n", username);
                                } else {
                                    fprintf(stderr, "\nCaso finger con utente %s con opzione -l\n\n", username);


                                    time_t lastLoginTime = 0;
                                    pwd = NULL;
                                    int user_info_printed = 0;
                                    char last_login[64];

                                    setutxent();

                                    while ((ut = getutxent()) != NULL) {

                                        //printf("1 - ut: %s\n", ut->ut_user);
                                        //printf("2 - Username: %s\n", username);

                                        if (ut->ut_type == USER_PROCESS && strcmp(ut->ut_user, username) == 0) {

                                            struct passwd *current_pwd = getpwnam(ut->ut_user);
                                            if (current_pwd != NULL) {
                                                //printf("3 - Current pwd: %s\n", current_pwd->pw_name);

                                                if (pwd == NULL) {
                                                    pwd = current_pwd;
                                                }
                                                //printf("4 - Pwd: %s\n", pwd->pw_name);

                                                if (ut->ut_tv.tv_sec > lastLoginTime) {
                                                    lastLoginTime = ut->ut_tv.tv_sec;
                                                }


                                                struct tm *tm_info = localtime(&lastLoginTime);
                                                strftime(last_login, sizeof(last_login), "%a %b %d %H:%M (%Z)", tm_info);
                                            }
                                        }
                                    }

                                    setutxent();  // Reset to beginning of utmpx entries

                                    while ((ut = getutxent()) != NULL) {
                                        if (ut->ut_type == USER_PROCESS && strcmp(ut->ut_user, username) == 0) {
                                            struct passwd *current_pwd = getpwnam(ut->ut_user);
                                            if (current_pwd != NULL && pwd != NULL && strcmp(current_pwd->pw_name, pwd->pw_name) == 0) {
                                                if (!user_info_printed) {
                                                    UserInfo userInfo = getUserInfo(ut, pwd);
                                                    printf("Login: %-33sName: %s\n", userInfo.login, userInfo.name);
                                                    printf("Directory: %-29sShell: %s\n", userInfo.directory, userInfo.shell);
                                                    user_info_printed = 1;
                                                }

                                                UserInfo userInfo = getUserInfo(ut, current_pwd);
                                                char *idleTime;
                                                if (strcmp(userInfo.tty, "console") == 0) {
                                                    idleTime = getCompleteIdle(ut->ut_tv.tv_sec);
                                                } else {
                                                    struct stat f_info;
                                                    char path[50] = "/dev/";
                                                    strcat(path, userInfo.tty);

                                                    if (stat(path, &f_info) != 0) {
                                                        perror("stat error");
                                                        continue;
                                                    }

                                                    idleTime = getCompleteIdle(f_info.st_mtime);
                                                }
                                                strncpy(userInfo.idle, idleTime, sizeof(userInfo.idle));
                                                printf("On since %s on %s,       %s ?(messages off)?\n", last_login, userInfo.tty, userInfo.idle);

                                            }
                                        }
                                    }

                                    if (pwd != NULL) {
                                        checkMailStatus(pwd->pw_name);
                                        checkPlanStatus(pwd->pw_dir);
                                    }

                                    /*
                                    while ((ut = getutxent()) != NULL) {
                                        if (ut->ut_type == USER_PROCESS && strcmp(ut->ut_user, username) == 0) {
                                            UserInfo userInfo = getUserInfo(ut, pwd);

                                            // Last login in formato Fri May 31 23:14 (CEST)
                                            time_t lastLoginTime = 0;

                                            if (ut->ut_tv.tv_sec > lastLoginTime) {
                                                lastLoginTime = ut->ut_tv.tv_sec;
                                            }

                                            char last_login[64];
                                            struct tm *tm_info = localtime(&lastLoginTime);
                                            strftime(last_login, sizeof(last_login), "%a %b %d %H:%M (%Z)", tm_info);



                                            printf("Login: %-33sName: %s\n", userInfo.login, userInfo.name);
                                            printf("Directory: %-29sShell: %s\n", userInfo.directory, userInfo.shell);

                                            printf("On since %s on %s,       idle ?%s? days ?? ?(messages off)?\n", last_login, userInfo.tty, userInfo.idle);

                                            checkMailStatus(pwd->pw_name); // Controlla e stampa il campo Mail
                                            checkPlanStatus(pwd->pw_dir); // Controlla e stampa il campo Plan


                                            break; // Una volta trovato l'utente, esci dal ciclo
                                        }
                                    }
                                    */

                                }

                            }
                            break;

                        case 's':

                            for (int i = 2; i < argc; i++) {

                                char *username = argv[i];
                                struct passwd *pwd = getpwnam(username);

                                if (pwd == NULL) {
                                    fprintf(stderr, "\nUtente %s non trovato!\n\n", username);
                                } else {
                                    fprintf(stderr, "\nCaso finger con utente %s con opzione -s\n\n", username);

                                    printf("%-17s%-17s%-9s%-6s%-7s%-7s%-8s%s\n", "Login", "Name", "TTY", "Idle", "Login", "Time", "Office", "Phone");

                                    while ((ut = getutxent()) != NULL) {
                                        if (ut->ut_type == USER_PROCESS && strcmp(ut->ut_user, username) == 0) {
                                            UserInfo userInfo = getUserInfo(ut, pwd);

                                            if (strcmp(userInfo.tty, "console") == 0) {
                                                strncpy(userInfo.tty, "*console", sizeof(userInfo.tty));
                                            } else {
                                                snprintf(userInfo.tty, sizeof(userInfo.tty), " %s", &ut->ut_line[3]);
                                            }

                                            printf("%-17s%-16s%-8s%6s  %-7s%-7s%-8s%s\n", userInfo.login, userInfo.name, userInfo.tty, userInfo.idle, userInfo.weekDay, userInfo.hoursMinutes, userInfo.officeLocation, userInfo.officePhone);
                                        }
                                    }
                                }
                            }
                            break;

                        case 'm':
                            for (int i = 2; i < argc; i++) {

                                char *username = argv[i];
                                struct passwd *pwd = getpwnam(username);

                                if (pwd == NULL) {
                                    fprintf(stderr, "\nUtente %s non trovato!\n\n", username);
                                } else {
                                    fprintf(stderr, "\nCaso finger con utente %s con opzione -m\n\n", username);


                                    time_t lastLoginTime = 0;
                                    pwd = NULL;
                                    int user_info_printed = 0;
                                    char last_login[64];

                                    setutxent();

                                    while ((ut = getutxent()) != NULL) {

                                        //printf("1 - ut: %s\n", ut->ut_user);
                                        //printf("2 - Username: %s\n", username);

                                        if (ut->ut_type == USER_PROCESS && strcmp(ut->ut_user, username) == 0) {

                                            struct passwd *current_pwd = getpwnam(ut->ut_user);
                                            if (current_pwd != NULL) {
                                                //printf("3 - Current pwd: %s\n", current_pwd->pw_name);

                                                if (pwd == NULL) {
                                                    pwd = current_pwd;
                                                }
                                                //printf("4 - Pwd: %s\n", pwd->pw_name);

                                                if (ut->ut_tv.tv_sec > lastLoginTime) {
                                                    lastLoginTime = ut->ut_tv.tv_sec;
                                                }


                                                struct tm *tm_info = localtime(&lastLoginTime);
                                                strftime(last_login, sizeof(last_login), "%a %b %d %H:%M (%Z)", tm_info);
                                            }
                                        }
                                    }

                                    setutxent();  // Reset to beginning of utmpx entries

                                    while ((ut = getutxent()) != NULL) {
                                        if (ut->ut_type == USER_PROCESS && strcmp(ut->ut_user, username) == 0) {
                                            struct passwd *current_pwd = getpwnam(ut->ut_user);
                                            if (current_pwd != NULL && pwd != NULL && strcmp(current_pwd->pw_name, pwd->pw_name) == 0) {
                                                if (!user_info_printed) {
                                                    UserInfo userInfo = getUserInfo(ut, pwd);
                                                    printf("Login: %-33sName: %s\n", userInfo.login, userInfo.name);
                                                    printf("Directory: %-29sShell: %s\n", userInfo.directory, userInfo.shell);
                                                    user_info_printed = 1;
                                                }

                                                UserInfo userInfo = getUserInfo(ut, current_pwd);
                                                char *idleTime;
                                                if (strcmp(userInfo.tty, "console") == 0) {
                                                    idleTime = getCompleteIdle(ut->ut_tv.tv_sec);
                                                } else {
                                                    struct stat f_info;
                                                    char path[50] = "/dev/";
                                                    strcat(path, userInfo.tty);

                                                    if (stat(path, &f_info) != 0) {
                                                        perror("stat error");
                                                        continue;
                                                    }

                                                    idleTime = getCompleteIdle(f_info.st_mtime);
                                                }
                                                strncpy(userInfo.idle, idleTime, sizeof(userInfo.idle));
                                                printf("On since %s on %s,       %s ?(messages off)?\n", last_login, userInfo.tty, userInfo.idle);

                                            }
                                        }
                                    }

                                    if (pwd != NULL) {
                                        checkMailStatus(pwd->pw_name);
                                        checkPlanStatus(pwd->pw_dir);
                                    }

                                    /*
                                    while ((ut = getutxent()) != NULL) {
                                        if (ut->ut_type == USER_PROCESS && strcmp(ut->ut_user, username) == 0) {
                                            UserInfo userInfo = getUserInfo(ut, pwd);

                                            // Last login in formato Fri May 31 23:14 (CEST)
                                            time_t lastLoginTime = 0;

                                            if (ut->ut_tv.tv_sec > lastLoginTime) {
                                                lastLoginTime = ut->ut_tv.tv_sec;
                                            }

                                            char last_login[64];
                                            struct tm *tm_info = localtime(&lastLoginTime);
                                            strftime(last_login, sizeof(last_login), "%a %b %d %H:%M (%Z)", tm_info);



                                            printf("Login: %-33sName: %s\n", userInfo.login, userInfo.name);
                                            printf("Directory: %-29sShell: %s\n", userInfo.directory, userInfo.shell);

                                            printf("On since %s on %s,       idle ?%s? days ?? ?(messages off)?\n", last_login, userInfo.tty, userInfo.idle);

                                            checkMailStatus(pwd->pw_name); // Controlla e stampa il campo Mail
                                            checkPlanStatus(pwd->pw_dir); // Controlla e stampa il campo Plan


                                            break; // Una volta trovato l'utente, esci dal ciclo
                                        }
                                    }
                                    */

                                }

                            }
                            break;

                        case 'p':
                            for (int i = 2; i < argc; i++) {

                                char *username = argv[i];
                                struct passwd *pwd = getpwnam(username);

                                if (pwd == NULL) {
                                    fprintf(stderr, "\nUtente %s non trovato!\n\n", username);
                                } else {
                                    fprintf(stderr, "\nCaso finger con utente %s con opzione -p\n\n", username);


                                    time_t lastLoginTime = 0;
                                    pwd = NULL;
                                    int user_info_printed = 0;
                                    char last_login[64];

                                    setutxent();

                                    while ((ut = getutxent()) != NULL) {

                                        //printf("1 - ut: %s\n", ut->ut_user);
                                        //printf("2 - Username: %s\n", username);

                                        if (ut->ut_type == USER_PROCESS && strcmp(ut->ut_user, username) == 0) {

                                            struct passwd *current_pwd = getpwnam(ut->ut_user);
                                            if (current_pwd != NULL) {
                                                //printf("3 - Current pwd: %s\n", current_pwd->pw_name);

                                                if (pwd == NULL) {
                                                    pwd = current_pwd;
                                                }
                                                //printf("4 - Pwd: %s\n", pwd->pw_name);

                                                if (ut->ut_tv.tv_sec > lastLoginTime) {
                                                    lastLoginTime = ut->ut_tv.tv_sec;
                                                }


                                                struct tm *tm_info = localtime(&lastLoginTime);
                                                strftime(last_login, sizeof(last_login), "%a %b %d %H:%M (%Z)", tm_info);
                                            }
                                        }
                                    }

                                    setutxent();  // Reset to beginning of utmpx entries

                                    while ((ut = getutxent()) != NULL) {
                                        if (ut->ut_type == USER_PROCESS && strcmp(ut->ut_user, username) == 0) {
                                            struct passwd *current_pwd = getpwnam(ut->ut_user);
                                            if (current_pwd != NULL && pwd != NULL && strcmp(current_pwd->pw_name, pwd->pw_name) == 0) {
                                                if (!user_info_printed) {
                                                    UserInfo userInfo = getUserInfo(ut, pwd);
                                                    printf("Login: %-33sName: %s\n", userInfo.login, userInfo.name);
                                                    printf("Directory: %-29sShell: %s\n", userInfo.directory, userInfo.shell);
                                                    user_info_printed = 1;
                                                }

                                                UserInfo userInfo = getUserInfo(ut, current_pwd);
                                                char *idleTime;
                                                if (strcmp(userInfo.tty, "console") == 0) {
                                                    idleTime = getCompleteIdle(ut->ut_tv.tv_sec);
                                                } else {
                                                    struct stat f_info;
                                                    char path[50] = "/dev/";
                                                    strcat(path, userInfo.tty);

                                                    if (stat(path, &f_info) != 0) {
                                                        perror("stat error");
                                                        continue;
                                                    }

                                                    idleTime = getCompleteIdle(f_info.st_mtime);
                                                }
                                                strncpy(userInfo.idle, idleTime, sizeof(userInfo.idle));
                                                printf("On since %s on %s,       %s ?(messages off)?\n", last_login, userInfo.tty, userInfo.idle);

                                            }
                                        }
                                    }

                                    if (pwd != NULL) {
                                        checkMailStatus(pwd->pw_name);
                                    }

                                    /*
                                    while ((ut = getutxent()) != NULL) {
                                        if (ut->ut_type == USER_PROCESS && strcmp(ut->ut_user, username) == 0) {
                                            UserInfo userInfo = getUserInfo(ut, pwd);

                                            // Last login in formato Fri May 31 23:14 (CEST)
                                            time_t lastLoginTime = 0;

                                            if (ut->ut_tv.tv_sec > lastLoginTime) {
                                                lastLoginTime = ut->ut_tv.tv_sec;
                                            }

                                            char last_login[64];
                                            struct tm *tm_info = localtime(&lastLoginTime);
                                            strftime(last_login, sizeof(last_login), "%a %b %d %H:%M (%Z)", tm_info);



                                            printf("Login: %-33sName: %s\n", userInfo.login, userInfo.name);
                                            printf("Directory: %-29sShell: %s\n", userInfo.directory, userInfo.shell);

                                            printf("On since %s on %s,       idle ?%s? days ?? ?(messages off)?\n", last_login, userInfo.tty, userInfo.idle);

                                            checkMailStatus(pwd->pw_name); // Controlla e stampa il campo Mail
                                            checkPlanStatus(pwd->pw_dir); // Controlla e stampa il campo Plan


                                            break; // Una volta trovato l'utente, esci dal ciclo
                                        }
                                    }
                                    */

                                }

                            }
                            break;

                        default:
                            printf("L'argomento %s non rientra nel caso di '-l', '-s', '-m' o '-p'.\n", argv[1]);
                            break;
                    }

                }

            } else {

                printf("Usage:  finger [-lmps] [user ...]\n");

            }
        } else {

            for (int i = 1; i < argc; i++) {

                char *username = argv[i];
                struct passwd *pwd = getpwnam(username);
                int user_found = 0;

                if (pwd == NULL) {
                    fprintf(stderr, "\nUtente %s non trovato!\n\n", username);
                } else {
                    fprintf(stderr, "\nCaso finger con utente %s senza opzioni [-lmsp]\n\n", username);

                    time_t lastLoginTime = 0;
                    pwd = NULL;
                    int user_info_printed = 0;
                    char last_login[64];

                    setutxent();

                    while ((ut = getutxent()) != NULL) {

                        //printf("1 - ut: %s\n", ut->ut_user);
                        //printf("2 - Username: %s\n", username);

                        if (ut->ut_type == USER_PROCESS && strcmp(ut->ut_user, username) == 0) {

                            struct passwd *current_pwd = getpwnam(ut->ut_user);
                            if (current_pwd != NULL) {
                                //printf("3 - Current pwd: %s\n", current_pwd->pw_name);

                                if (pwd == NULL) {
                                    pwd = current_pwd;
                                }
                                //printf("4 - Pwd: %s\n", pwd->pw_name);

                                if (ut->ut_tv.tv_sec > lastLoginTime) {
                                    lastLoginTime = ut->ut_tv.tv_sec;
                                }


                                struct tm *tm_info = localtime(&lastLoginTime);
                                strftime(last_login, sizeof(last_login), "%a %b %d %H:%M (%Z)", tm_info);
                            }
                        }
                    }

                    setutxent();  // Reset to beginning of utmpx entries

                    while ((ut = getutxent()) != NULL) {
                        if (ut->ut_type == USER_PROCESS && strcmp(ut->ut_user, username) == 0) {
                            struct passwd *current_pwd = getpwnam(ut->ut_user);
                            if (current_pwd != NULL && pwd != NULL && strcmp(current_pwd->pw_name, pwd->pw_name) == 0) {
                                if (!user_info_printed) {
                                    UserInfo userInfo = getUserInfo(ut, pwd);
                                    printf("Login: %-33sName: %s\n", userInfo.login, userInfo.name);
                                    printf("Directory: %-29sShell: %s\n", userInfo.directory, userInfo.shell);
                                    user_info_printed = 1;
                                }

                                UserInfo userInfo = getUserInfo(ut, current_pwd);
                                char *idleTime;
                                if (strcmp(userInfo.tty, "console") == 0) {
                                    idleTime = getCompleteIdle(ut->ut_tv.tv_sec);
                                } else {
                                    struct stat f_info;
                                    char path[50] = "/dev/";
                                    strcat(path, userInfo.tty);

                                    if (stat(path, &f_info) != 0) {
                                        perror("stat error");
                                        continue;
                                    }

                                    idleTime = getCompleteIdle(f_info.st_mtime);
                                }
                                strncpy(userInfo.idle, idleTime, sizeof(userInfo.idle));
                                printf("On since %s on %s,       %s ?(messages off)?\n", last_login, userInfo.tty, userInfo.idle);

                            }
                        }
                    }

                    if (pwd != NULL) {
                        checkMailStatus(pwd->pw_name);
                        checkPlanStatus(pwd->pw_dir);
                    }


                    /*
                    if (!user_found) {
                        // Utente inattivo, stampa comunque le informazioni richieste
                        time_t last_login_time = 0;
                        char last_login_tty[32] = "unknown";

                        printf("Login: %-33sName: %s\n", username, pwd->pw_gecos);
                        printf("Directory: %-29sShell: %s\n", pwd->pw_dir, pwd->pw_shell);

                        printf("%d\n", getLastLoginUnloggedUser(username, &last_login_time, last_login_tty, sizeof(last_login_tty)));

                        if (getLastLoginUnloggedUser(username, &last_login_time, last_login_tty, sizeof(last_login_tty)) == 1) {
                            char last_login[64];
                            struct tm *tm_info = localtime(&last_login_time);
                            strftime(last_login, sizeof(last_login), "%a %b %d %H:%M (%Z)", tm_info);

                            printf("Last login %s on %s\n", last_login, last_login_tty);
                        } else {
                            printf("Last login not found\n");
                        }


                        checkMailStatus(pwd->pw_name); // Controlla e stampa il campo Mail
                        checkPlanStatus(pwd->pw_dir); // Controlla e stampa il campo Plan
                    }
                     */

                }

            }

        }

    }

    printf("----------------------------------------------------------------------------------------------------\n");

    // Close the utmpx file
    endutxent();

    return 0;
}