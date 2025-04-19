#include <mpi.h>
#include <iostream>
#include <chrono>
#include <string>

using namespace std;

const string brute_chars = "abcdefghijklmnopqrstuvwxyz";
const int MAX_LEN = 5;
const string correct_password = "queue";

// Перетворює число у рядок за основою brute_chars.size()
string number_to_word(uint64_t number, int length, size_t charset_size) {
    string word(length, ' ');
    for (int i = length - 1; i >= 0; --i) {
        word[i] = brute_chars[number % charset_size];
        number /= charset_size;
    }
    return word;
}

// Перевірка пароля
bool check_password(const string& guess) {
    return guess == correct_password;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int world_size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size); // Кількість процесів
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);       // Поточний процес

    size_t charset_size = brute_chars.size();
    uint64_t total = 1;

    for (int i = 0; i < MAX_LEN; ++i)
        total *= charset_size;

    bool found = false;
    string result;
    uint64_t local_index = 0;

    auto start = chrono::steady_clock::now();

    // Основний пошук: кожен процес бере свої ітерації залежно від рангу
    for (uint64_t i = rank; i < total; i += world_size) {
        if (found)
            break;

        string guess = number_to_word(i, MAX_LEN, charset_size);

        if (check_password(guess)) {
            result = guess;
            found = true;

            // Сповіщаємо інші процеси, що пароль знайдено
            for (int p = 0; p < world_size; ++p) {
                if (p != rank)
                    MPI_Send(&i, 1, MPI_UINT64_T, p, 0, MPI_COMM_WORLD);
            }
            break;
        }

        // Перевірка повідомлень про знаходження пароля іншими
        int flag;
        MPI_Status status;
        MPI_Iprobe(MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &flag, &status);
        if (flag) {
            uint64_t dummy;
            MPI_Recv(&dummy, 1, MPI_UINT64_T, status.MPI_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            found = true;
            break;
        }
    }

    auto end = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    // Виводимо результат тільки з одного процесу
    if (found && !result.empty()) {
        cout << "Process " << rank << " found the password: " << result << endl;
        cout << "Elapsed time: " << elapsed / 1000.0 << " seconds" << endl;
    }

    MPI_Finalize();
    return 0;
}
