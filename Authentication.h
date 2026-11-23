#ifndef RAR_AUTHENTICATION_H                    // Header guard: stops compiler from including this file twice
#define RAR_AUTHENTICATION_H                    // Marks this file as already included

#include <cstdint>                              // Gives us fixed-size integers (uint32_t, uint64_t)
#include <iomanip>                              // For formatting hex output (setw, setfill)
#include <iostream>                             // For input/output (std::cin, std::cout)
#include <sstream>                              // For building strings like cout (ostringstream)
#include <string>                               // For std::string (text)

namespace rar {                                 // Main project namespace (matches other files)
namespace auth_detail {                         // Inner namespace to hide helper security functions

// Rotates the bits of a 32-bit number 'x' to the right by 'n' positions
inline std::uint32_t rotr(std::uint32_t x, unsigned n) {
    return (x >> n) | (x << (32 - n));          // Shift right, wrap fallen bits around to the left
}

// SHA-256: turns any text into a fixed 64-character scrambled code (hash)
// Note: You do NOT need to memorize the hex numbers—they are standard SHA-256 constants
inline std::string sha256(const std::string& input) {
    // 64 standard fixed constants defined by the SHA-256 math formula
    static const std::uint32_t k[64] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
    };

    // STEP 1: Padding (make message length a multiple of 64 bytes)
    std::string msg = input;                    // Copy the input text
    const std::uint64_t bitLen = static_cast<std::uint64_t>(msg.size()) * 8; // Length in bits
    msg.push_back(static_cast<char>(0x80));     // Add a '1' bit (0x80) at the end
    while ((msg.size() % 64) != 56)             // Fill with 0s until 8 bytes remain in the 64-byte block
        msg.push_back('\0');
    for (int i = 7; i >= 0; --i)                // Append original bit length as 8 bytes at the end
        msg.push_back(static_cast<char>((bitLen >> (i * 8)) & 0xff));

    // STEP 2: 8 initial starting values (standard SHA-256 constants)
    std::uint32_t h[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    // STEP 3: Process the message in 64-byte chunks
    for (std::size_t off = 0; off < msg.size(); off += 64) {
        std::uint32_t w[64];                    // Array of 64 words for this chunk

        // Break the 64-byte chunk into 16 32-bit numbers
        for (int i = 0; i < 16; ++i) {
            const unsigned char* p = reinterpret_cast<const unsigned char*>(msg.data() + off + i * 4);
            w[i] = (std::uint32_t(p[0]) << 24) | (std::uint32_t(p[1]) << 16)
                 | (std::uint32_t(p[2]) << 8)  | p[3];
        }

        // Expand those 16 numbers into 64 scrambled numbers
        for (int i = 16; i < 64; ++i) {
            const std::uint32_t s0 = rotr(w[i-15], 7) ^ rotr(w[i-15], 18) ^ (w[i-15] >> 3);
            const std::uint32_t s1 = rotr(w[i-2], 17) ^ rotr(w[i-2], 19)  ^ (w[i-2] >> 10);
            w[i] = w[i-16] + s0 + w[i-7] + s1;
        }

        // Copy current hash state into working variables a..hh
        std::uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
        std::uint32_t e = h[4], f = h[5], g = h[6], hh = h[7];

        // Mix all 64 rounds together using bitwise math
        for (int i = 0; i < 64; ++i) {
            const std::uint32_t s1  = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
            const std::uint32_t ch  = (e & f) ^ (~e & g);
            const std::uint32_t t1  = hh + s1 + ch + k[i] + w[i];
            const std::uint32_t s0  = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
            const std::uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            const std::uint32_t t2  = s0 + maj;

            hh = g; g = f; f = e; e = d + t1;   // Shift values down
            d = c;  c = b; b = a; a = t1 + t2;  // Put new mixed values at the top
        }

        // Add this chunk's result back into the main hash h[0..7]
        h[0]+=a; h[1]+=b; h[2]+=c; h[3]+=d; h[4]+=e; h[5]+=f; h[6]+=g; h[7]+=hh;
    }

    // STEP 4: Convert the 8 numbers into a 64-character hexadecimal string
    std::ostringstream out;
    for (int i = 0; i < 8; ++i)
        out << std::hex << std::setw(8) << std::setfill('0') << h[i];
    return out.str();                           // Return final SHA-256 hash
}

// Makes password harder to crack by adding secret text ("salt") and hashing 100,000 times
inline std::string passwordDigest(const std::string& password) {
    std::string value = sha256(std::string("RAR-local-auth-v1:") + password); // First salted hash
    for (int i = 0; i < 100000; ++i)            // Repeat hashing 100,000 times to slow down hackers
        value = sha256(value + "#RAR#");
    return value;                               // Return final scrambled password
}

// Safely compares two strings so hackers cannot guess the password by measuring time
inline bool constantTimeEqual(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;     // Different lengths cannot be equal
    unsigned char diff = 0;                     // Tracks if any character is different
    for (std::size_t i = 0; i < a.size(); ++i)
        diff |= static_cast<unsigned char>(a[i] ^ b[i]); // XOR is 0 if same, non-zero if different
    return diff == 0;                           // True only if every single character matched
}

} // namespace auth_detail

// =======================================================
// MAIN LOGIN FUNCTION (Called from main.cpp at startup)
// =======================================================
inline bool authenticate() {
    static const std::string expectedId = "Anshuman"; // Correct username (case-sensitive)
    // Stored hash of the real password (real password is never written in plain text)
    static const std::string expectedPasswordDigest =
        "78f3ae0c87fb67947e203c5decf093058f50a7e3b528f99cf58a44fca010a94f";

    // Give the user 3 chances to log in
    for (int attempt = 1; attempt <= 3; ++attempt) {
        std::string id, password;               // Variables to store user input

        std::cout << "\n====================================\n"
                  << "       REASON-ACT-REFLECT LOGIN\n"
                  << "====================================\n"
                  << "User ID: ";
        if (!(std::cin >> id)) return false;    // Read User ID; exit if input fails

        std::cout << "Password: ";
        if (!(std::cin >> password)) return false; // Read Password; exit if input fails

        // Check if both User ID and hashed Password match
        const bool idOk = (id == expectedId);
        const bool passwordOk = auth_detail::constantTimeEqual(
            auth_detail::passwordDigest(password), expectedPasswordDigest
        );

        if (idOk && passwordOk) {               // Both correct -> allow access
            std::cout << "\nLogin successful. Welcome, " << expectedId << "!\n\n";
            return true;
        }

        // Wrong login -> show remaining attempts
        std::cout << "\nAccess denied. Incorrect ID or password. Attempts remaining: "
                  << (3 - attempt) << "\n";
    }

    // All 3 attempts failed -> close app
    std::cout << "Too many failed attempts. Closing application.\n";
    return false;
}

} // namespace rar
#endif                                          // End of header guard