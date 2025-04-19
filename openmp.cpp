#include <iostream>
#include <chrono>
#include <string>
#include <omp.h>
#include <vector>

using namespace std;

const string brute_chars = "abcdefghijklmnopqrstuvwxyz";
const int MAX_LEN = 5;
const string correct_password = "queue";

volatile bool password_found = false;
string result = "";

bool check_password(const string& guess) {
    return guess == correct_password;
}

// Перетворюємо число в комбінацію символів за основою brute_chars.size()
string number_to_word(uint64_t number, int length, size_t charset_size) {
    string word(length, ' ');
    for (int i = length - 1; i >= 0; --i) {
        word[i] = brute_chars[number % charset_size];
        number /= charset_size;
    }
    return word;
}

void brute_force_parallel() {
    size_t charset_size = brute_chars.size();
    uint64_t total = 1;

    // Обчислюємо загальну кількість комбінацій довжини MAX_LEN
    for (int i = 0; i < MAX_LEN; ++i)
        total *= charset_size;

#pragma omp parallel for num_threads(4)
    for (long long int i = 0; i < total; ++i) {
        if (password_found)
            continue;

        string guess = number_to_word(i, MAX_LEN, charset_size);

        if (check_password(guess)) {
                if (!password_found) {
                    password_found = true;
#pragma omp flush(password_found)
                    result = guess;
                }
        }
    }

    if (password_found) {
        cout << "Correct password is: " << result << endl;
    }
    else {
        cout << "Password was not found." << endl;
    }
}

int main() {
    auto start = chrono::steady_clock::now();

    brute_force_parallel();

    auto end = chrono::steady_clock::now();
    auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start).count();

    cout << "Elapsed time: " << elapsed / 1000.0 << " seconds" << endl;

    return 0;
}
