# Reason-Act-Reflect — Expanded C++ Prototype

This version is organized into 11 source/header files and emphasizes OOP and data structures.

## Files
1. `Authentication.h` — login gate (ID check and salted, repeated password digest).
2. `Config.h` — paths and application settings.
3. `Document.h` — encapsulated document model.
4. `Tokenizer.h` — normalization, tokenization, stop words.
5. `SearchIndex.h` — inverted index and postings lists.
6. `Ranker.h` — IDF-based scoring and ranked results.
7. `FeedbackLearner.h` — feedback-based heuristic weight updates.
8. `StorageManager.h` — local persistence and loading.
9. `SearchEngine.h` — document CRUD, retrieval, indexing, history.
10. `Agent.h` — menu and Reason–Act–Reflect interaction.
11. `main.cpp` — program entry point.

## Build
Designed to compile with C++14-capable GCC/MinGW (and newer compilers). The storage code does not depend on `<filesystem>`, so it also avoids the common older-MinGW `filesystem: No such file or directory` error.

```bash
g++ -std=c++14 -O2 -Wall -Wextra main.cpp -o reason_act_reflect.exe
```

Run Linux/macOS:
```bash
./reason_act_reflect
```

Run Windows PowerShell (from this folder):
```powershell
.\reason_act_reflect.exe
```

On launch, the application first asks for a login. The configured login ID is `Anshuman`; the password is the one supplied by the project owner. After three failed attempts, the application exits. The password is not stored as plain text in the source; a salted, repeated SHA-256 digest is embedded instead.

The application creates a `data/` folder in its current working directory. It stores:
- `documents.db` — document records
- `feedback.db` — ranking weights and document relevance boosts
- `search_history.db` — recent query history

## Data structures and OOP used
- Encapsulation and composition across `Document`, `SearchIndex`, `Ranker`, `FeedbackLearner`, `StorageManager`, `SearchEngine`, and `Agent`
- `vector` for documents, tokens, results, and history
- `unordered_map` for inverted index postings, term frequencies, weights, and feedback
- `unordered_set` for candidate IDs and unique terms
- `priority`-style ranked retrieval through sorting by score
- STL algorithms for sorting, filtering, and lookup

## Important scope note
This is a larger modular 70% prototype, not a 10,000-line production AI system. It implements local keyword retrieval and heuristic feedback. It does not yet implement embeddings, an LLM, semantic vector search, full RAG generation, PDF/DOCX parsing, data-at-rest encryption, or production-grade security. The built-in login is a basic local access gate, not strong protection against a person who can inspect or patch the executable. For sensitive or distributed deployments, use OS-level accounts/file permissions and a vetted password KDF such as Argon2id or bcrypt; do not reuse a personal password. Feedback adjusts ranking weights; it is not neural-network training. Do not put sensitive documents in the prototype until suitable access controls and encryption are added.
