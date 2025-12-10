#include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <ctype.h> // New include for isdigit()
    #include <unistd.h> // For sleep (optional, for visual effect)

    // --- Configuration and Constants ---
    #define MAX_STUDENTS 100
    #define FILENAME "student_records.dat"
    #define MAX_NAME_LEN 50
    #define MAX_COURSE_LEN 30
    #define MAX_SUBJECTS_LEN 100

    // --- ANSI Escape Codes for Styling (Mimicking a Website Look) ---
    #define RESET     "\033[0m"
    #define BOLD      "\033[1m"
    #define RED       "\033[31m"
    #define GREEN     "\033[32m"
    #define YELLOW    "\033[33m"
    #define BLUE      "\033[34m"
    #define MAGENTA   "\033[35m"
    #define CYAN      "\033[36m"
    #define WHITE     "\033[37m"
    #define BG_BLUE   "\033[44m"
    #define BG_CYAN   "\033[46m" 

    // --- Global Variables ---
    int student_count = 0;

    // --- Structure Definition ---
    typedef struct {
        char admn_no[14];         // APxxxxxxxxxxx (13 chars + null terminator)
        char name[MAX_NAME_LEN];
        int age;
        char course[MAX_COURSE_LEN];
        char subjects[MAX_SUBJECTS_LEN]; // e.g., "Math, Physics, English"
        float gpa;                // Out of 10.00
        float attendance_perc;    // Total attendance percentage
        int year_of_study;
    } Student;

    Student records[MAX_STUDENTS];

    // --- Utility Functions for Presentation ---

    /**
     * @brief Prints a colored, centered, boxed header for the application.
     */
    void print_header(const char *title) {
        int width = 80;
        int len = strlen(title);
        int padding = (width - len - 2) / 2;

        printf("\n");
        printf(BOLD BG_BLUE WHITE);
        for (int i = 0; i < width; i++) printf(" ");
        printf("\r");
        printf(" %*s%s%*s " RESET, padding, "", title, width - len - 2 - padding, "");
        printf(RESET "\n");
        printf("\n");
    }

    /**
     * @brief Draws a horizontal separator line.
     */
    void print_separator() {
        printf(CYAN);
        for (int i = 0; i < 80; i++) printf("-");
        printf(RESET "\n");
    }

    /**
     * @brief Prints a colorful menu option.
     */
    void print_menu_option(int number, const char *description) {
        printf(CYAN BOLD "  [%d]" RESET WHITE " %s\n" RESET, number, description);
    }

    /**
     * @brief Flushes output, waits for ENTER key, and clears the screen.
     * This ensures the user has time to read the output before the menu redraws.
     */
    void pause_and_clear() {
        fflush(stdout); // Ensure all previous output is immediately rendered
        printf(YELLOW "\nPress ENTER to continue..." RESET);
        while (getchar() != '\n'); // Wait for ENTER key
        printf("\033[H\033[J"); // Clear screen
    }

    // --- Validation Functions ---

    /**
     * @brief Validates the Admission Number format (AP followed by 11 digits) using standard C functions.
     * @param admn_no The admission number string.
     * @return 1 on success (valid format), 0 on failure.
     */
    int is_valid_admn_no(const char *admn_no) {
        // Expected format: AP + 11 digits = 13 characters total
        if (strlen(admn_no) != 13) {
            return 0;
        }

        // Check first two characters: Must be 'A' and 'P'
        // isupper() check is slightly safer, but direct comparison is fine for fixed 'AP'
        if (admn_no[0] != 'A' || admn_no[1] != 'P') {
            return 0;
        }

        // Check the remaining 11 characters: Must all be digits
        for (int i = 2; i < 13; i++) {
            // isdigit() is defined in <ctype.h>
            if (!isdigit(admn_no[i])) {
                return 0;
            }
        }

        return 1; // All checks passed
    }

    // --- File Handling Functions (Persistence) ---

    /**
     * @brief Loads student records from the binary file.
     */
    void load_records() {
        FILE *file = fopen(FILENAME, "rb");
        student_count = 0;

        if (file == NULL) {
            printf(YELLOW "\nDatabase file '%s' not found. Starting with an empty record list.\n" RESET, FILENAME);
            return;
        }

        // Read the entire array in one go
        size_t read_count = fread(records, sizeof(Student), MAX_STUDENTS, file);
        student_count = (int)read_count;

        fclose(file);
        printf(GREEN BOLD "\nSuccessfully loaded %d records from the database.\n" RESET, student_count);
    }

    /**
     * @brief Saves current student records to the binary file.
     */
    void save_records() {
        FILE *file = fopen(FILENAME, "wb");

        if (file == NULL) {
            printf(RED BOLD "\nERROR: Could not open file '%s' for writing. Records not saved.\n" RESET, FILENAME);
            return;
        }

        // Write only the current number of active students
        fwrite(records, sizeof(Student), student_count, file);

        fclose(file);
        printf(GREEN BOLD "\nSuccessfully saved %d records to the database.\n" RESET, student_count);
    }

    /**
     * @brief Deletes the persistent file and clears all in-memory records.
     */
    void delete_all_records() {
        print_header("DELETE ALL RECORDS");

        printf(RED BOLD "\n  WARNING: This action is permanent and cannot be undone.\n" RESET);
        printf(YELLOW "  Are you sure you want to delete ALL student records? (Type 'YES' to confirm): " RESET);
        
        char confirmation[10];
        // We use scanf here to read a single token, which is appropriate for a simple confirmation
        if (scanf("%9s", confirmation) != 1) {
            printf(RED BOLD "\n  Confirmation failed. Operation cancelled.\n" RESET);
            // Clear the input buffer
            while (getchar() != '\n');
            pause_and_clear();
            return;
        }
        // Clear the rest of the line after reading the integer
        while (getchar() != '\n');

        if (strcmp(confirmation, "YES") == 0) {
            // 1. Clear in-memory data
            student_count = 0;
            
            // 2. Delete the persistent file using the remove() function
            if (remove(FILENAME) == 0) {
                printf(GREEN BOLD "\n  [SUCCESS] All in-memory records cleared and file '%s' successfully deleted.\n" RESET, FILENAME);
            } else {
                // Error could mean the file didn't exist, which is fine, or a permissions issue
                if (remove(FILENAME) == -1) {
                    // Check if file exists to give a more specific message
                    FILE *file = fopen(FILENAME, "r");
                    if (file != NULL) {
                        fclose(file);
                        printf(RED BOLD "\n  [ERROR] File '%s' exists but could not be deleted (Permission issue?).\n" RESET, FILENAME);
                    } else {
                        printf(YELLOW BOLD "\n  [INFO] File '%s' did not exist. Records cleared from memory.\n" RESET, FILENAME);
                    }
                } else {
                    printf(GREEN BOLD "\n  [SUCCESS] All records cleared from memory and file handled.\n" RESET);
                }
            }
        } else {
            printf(YELLOW BOLD "\n  Confirmation failed. Operation cancelled.\n" RESET);
        }
        
        pause_and_clear();
    }


    // --- Core Application Functions ---

    /**
     * @brief Adds a new student record after prompting the user for details.
     */
    void add_record() {
        print_header("ADD NEW STUDENT RECORD");

        if (student_count >= MAX_STUDENTS) {
            printf(RED BOLD "  Database capacity reached (%d students). Cannot add more records.\n" RESET, MAX_STUDENTS);
            pause_and_clear();
            return;
        }

        Student new_student;
        char buffer[256];

        // 1. Admission Number
        printf(YELLOW "  Enter Admission Number (APxxxxxxxxxxx): " RESET);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) { pause_and_clear(); return; }
        buffer[strcspn(buffer, "\n")] = 0; // Remove newline
        strncpy(new_student.admn_no, buffer, 13);
        new_student.admn_no[13] = '\0';

        if (!is_valid_admn_no(new_student.admn_no)) {
            printf(RED BOLD "  Invalid Admission Number format. Must be AP followed by 11 digits.\n" RESET);
            pause_and_clear();
            return;
        }

        // Check for duplicate
        for (int i = 0; i < student_count; i++) {
            if (strcmp(records[i].admn_no, new_student.admn_no) == 0) {
                printf(RED BOLD "  Record with Admission Number %s already exists.\n" RESET, new_student.admn_no);
                pause_and_clear();
                return;
            }
        }

        // 2. Name
        printf(YELLOW "  Enter Student Name: " RESET);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) { pause_and_clear(); return; }
        buffer[strcspn(buffer, "\n")] = 0;
        strncpy(new_student.name, buffer, MAX_NAME_LEN - 1);
        new_student.name[MAX_NAME_LEN - 1] = '\0';

        // 3. Age (Input validation loop)
        do {
            printf(YELLOW "  Enter Age (16-99): " RESET);
            if (fgets(buffer, sizeof(buffer), stdin) == NULL) { pause_and_clear(); return; }
            new_student.age = atoi(buffer);
        } while (new_student.age < 16 || new_student.age > 99);

        // 4. Course/Major
        printf(YELLOW "  Enter Course/Major: " RESET);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) { pause_and_clear(); return; }
        buffer[strcspn(buffer, "\n")] = 0;
        strncpy(new_student.course, buffer, MAX_COURSE_LEN - 1);
        new_student.course[MAX_COURSE_LEN - 1] = '\0';

        // 5. Subjects Opted
        printf(YELLOW "  Enter Subjects Opted (Comma separated): " RESET);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) { pause_and_clear(); return; }
        buffer[strcspn(buffer, "\n")] = 0;
        strncpy(new_student.subjects, buffer, MAX_SUBJECTS_LEN - 1);
        new_student.subjects[MAX_SUBJECTS_LEN - 1] = '\0';

        // 6. GPA (Input validation loop)
        do {
            printf(YELLOW "  Enter GPA (0.00-10.00): " RESET);
            if (fgets(buffer, sizeof(buffer), stdin) == NULL) { pause_and_clear(); return; }
            new_student.gpa = atof(buffer);
        } while (new_student.gpa < 0.0 || new_student.gpa > 10.0);

        // 7. Attendance Percentage (Input validation loop)
        do {
            printf(YELLOW "  Enter Attendance Percentage (0.0-100.0): " RESET);
            if (fgets(buffer, sizeof(buffer), stdin) == NULL) { pause_and_clear(); return; }
            new_student.attendance_perc = atof(buffer);
        } while (new_student.attendance_perc < 0.0 || new_student.attendance_perc > 100.0);

        // 8. Year of Study (Input validation loop)
        do {
            printf(YELLOW "  Enter Year of Study (1-4): " RESET);
            if (fgets(buffer, sizeof(buffer), stdin) == NULL) { pause_and_clear(); return; }
            new_student.year_of_study = atoi(buffer);
        } while (new_student.year_of_study < 1 || new_student.year_of_study > 4);


        // Add the new student to the array
        records[student_count] = new_student;
        student_count++;

        printf(GREEN BOLD "\n  [SUCCESS] Record for %s added successfully! Current total: %d\n" RESET, new_student.name, student_count);

        // Prompt to save immediately
        printf(YELLOW "  Do you want to save the changes to file now? (Y/N): " RESET);
        char choice;
        if (scanf(" %c", &choice) == 1 && (choice == 'Y' || choice == 'y')) {
            save_records();
        }
        // Clear the input buffer
        while (getchar() != '\n');
        
        pause_and_clear();
    }

    /**
     * @brief Displays all student records in a structured, neat table format.
     */
    void view_records() {
        print_header("ALL STUDENT RECORDS");

        if (student_count == 0) {
            printf(YELLOW BOLD "  No records found in the database. Add a new record first (Option 1).\n" RESET);
            pause_and_clear();
            return;
        }

        // --- Table Header ---
        print_separator();
        // ALIGNMENT FIX: The COURSE column width is increased from 10 to 20 to prevent data overflow.
        printf(BOLD BG_CYAN WHITE); 
        printf("| %-13s | %-20s | %-4s | %-20s | %-4s | %-6s | %-11s |\n" RESET,
            "ADMN NO.", "NAME", "AGE", "COURSE", "YR", "GPA", "ATTENDANCE");
        print_separator();

        // --- Table Rows ---
        for (int i = 0; i < student_count; i++) {
            Student *s = &records[i];
            // The COURSE column width is 20.
            // Numerical fields (AGE, YR, GPA, ATTENDANCE) are now RIGHT-JUSTIFIED 
            // by removing the '-' flag, which ensures the decimal points and '%' symbol align correctly.
            printf("| %-13s | %-20s | %4d | %-20s | %4d | %6.2f | %10.2f%% |\n",
                s->admn_no, s->name, s->age, s->course, s->year_of_study, s->gpa, s->attendance_perc);
        }

        // --- Table Footer ---
        print_separator();
        printf(BOLD WHITE "\n  Total Records: %d\n" RESET, student_count);
        
        pause_and_clear();
    }

    /**
     * @brief Searches for a student by Admission Number and displays details.
     */
    void search_record() {
        print_header("SEARCH STUDENT RECORD");
        char search_admn_no[14];
        printf(YELLOW "  Enter Admission Number to search (APxxxxxxxxxxx): " RESET);
        if (fgets(search_admn_no, sizeof(search_admn_no), stdin) == NULL) { pause_and_clear(); return; }
        search_admn_no[strcspn(search_admn_no, "\n")] = 0;

        if (!is_valid_admn_no(search_admn_no)) {
            printf(RED BOLD "  Invalid Admission Number format.\n" RESET);
            pause_and_clear();
            return;
        }

        for (int i = 0; i < student_count; i++) {
            if (strcmp(records[i].admn_no, search_admn_no) == 0) {
                Student *s = &records[i];

                printf(GREEN BOLD "\n  [MATCH FOUND]\n" RESET);
                print_separator();

                printf(BOLD WHITE "  Admission No: " RESET CYAN "%s\n" RESET, s->admn_no);
                printf(BOLD WHITE "  Name:         " RESET "%s\n", s->name);
                printf(BOLD WHITE "  Age:          " RESET "%d\n", s->age);
                printf(BOLD WHITE "  Course/Major: " RESET "%s\n", s->course);
                printf(BOLD WHITE "  Year of Study:" RESET "%d\n", s->year_of_study);
                printf(BOLD WHITE "  Subjects:     " RESET "%s\n", s->subjects);
                printf(BOLD WHITE "  GPA:          " RESET YELLOW "%.2f / 10.00\n" RESET, s->gpa);
                printf(BOLD WHITE "  Attendance:   " RESET MAGENTA "%.2f%%\n" RESET, s->attendance_perc);

                print_separator();
                pause_and_clear(); // Pause to allow reading the result
                return;
            }
        }

        printf(RED BOLD "\n  [NOT FOUND] No student found with Admission Number: %s\n" RESET, search_admn_no);
        pause_and_clear(); // Pause to allow reading the result
    }

    /**
     * @brief Displays the main application menu.
     */
    void display_menu() {
        print_header("STUDENT RECORD MANAGEMENT SYSTEM");

        printf(BOLD WHITE "  Welcome! Select an option from the menu below:\n\n" RESET);

        print_menu_option(1, "Add New Student Record");
        print_menu_option(2, "View All Student Records");
        print_menu_option(3, "Search Record by Admission Number");
        print_menu_option(4, "Save Records to File");
        print_menu_option(5, "DELETE ALL RECORDS (Start Fresh)");
        print_menu_option(6, "Exit Application (Unsaved data will be lost!)");

        print_separator();
        printf(YELLOW "  Enter your choice: " RESET);
    }

    // --- Main Function ---

    int main() {
        // Attempt to load existing data immediately on startup
        load_records();
        // Clear screen for a neat start (optional)
        printf("\033[H\033[J");

        int choice;

        while (1) {
            display_menu();

            if (scanf("%d", &choice) != 1) {
                printf(RED BOLD "\n  Invalid input. Please enter a number.\n" RESET);
                // Clear the input buffer for the next loop iteration
                while (getchar() != '\n');
                sleep(1);
                printf("\033[H\033[J"); // Clear screen after error message
                continue;
            }
            // Clear the rest of the line after reading the integer
            while (getchar() != '\n');

            // Clear screen before executing the action, ensuring the action's output is clean and visible
            printf("\033[H\033[J");
            
            switch (choice) {
                case 1:
                    add_record();
                    break;
                case 2:
                    view_records();
                    break;
                case 3:
                    search_record();
                    break;
                case 4:
                    save_records();
                    // We add a manual pause here since save_records doesn't use pause_and_clear
                    // due to its internal Y/N input loop logic.
                    pause_and_clear();
                    break;
                case 5:
                    delete_all_records();
                    break;
                case 6:
                    printf(MAGENTA BOLD "\n  Thank you for using the Student Management System. Goodbye!\n" RESET);
                    return 0;
                default:
                    printf(RED BOLD "\n  Invalid choice. Please enter a number between 1 and 6.\n" RESET);
                    pause_and_clear();
                    break;
            }
        }

        return 0;
    }
