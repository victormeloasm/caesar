# 📜 Caesar Cipher Tool — Teaching Edition

A tiny, fast, C++23 Caesar-cipher tool created by **Víctor Duarte Melo** as the opening exercise for his upcoming cryptography classes.

The goal is simple:
Help students understand how substitution ciphers work, why they are insecure, and how real-world cryptanalysis uses **frequency analysis** and **linguistic patterns** to break them.

This tool includes:

* ✔️ Caesar encryption
* ✔️ Caesar decryption
* ✔️ A **random-shift** mode (you *won’t* know the key)
* ✔️ A **PT-BR language-aware cracker** that guesses the shift using statistics + semantic scoring
* ✔️ Clean, portable C++23 + clang++/lld build

Minimal, sharp, and perfect for students to dissect.

---

## 🔧 Building

```bash
clang++ -std=c++23 -O3 -march=native -mtune=native -flto -fuse-ld=lld \
  -Wall -Wextra -Wpedantic \
  cesar_pt.cpp -o cesar
```

---

## 🚀 Usage

### Encrypt with a fixed shift

```bash
echo "hello world" | ./cesar --encrypt 7
```

### Decrypt with a fixed shift

```bash
echo "olssv dvysk" | ./cesar --decrypt 7
```

### Encrypt with a **random** shift

(shift is not displayed — this is intentional)

```bash
echo "hello world" | ./cesar --encrypt-random
```

### Crack an encrypted message

(uses frequency stats + semantic weighting)

```bash
echo "Khoor zruog" | ./cesar --crack
```

---

## 🎓 Why this project exists

This small tool is the **first cryptography exercise** in Víctor’s course.

Students will:

1. Modify the cipher
2. Improve the cracker
3. Add new heuristics or statistics
4. Realize how trivially Caesar can be broken
5. Use that insight to appreciate **modern cryptography**

It’s intentionally simple — because the hard part is *thinking*.

---

## 📄 License

**MIT License**

Do whatever you want: learn, break, rewrite, extend — just don’t blame us if your friend encrypts their homework with Caesar and loses the key.

