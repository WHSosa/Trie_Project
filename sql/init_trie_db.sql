-- sql/init_trie_db.sql
-- Creates table trie_words for the project (option B schema)

CREATE TABLE IF NOT EXISTS trie_words (
    word TEXT PRIMARY KEY,
    frequency INT DEFAULT 1,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

-- Optional: insert a sample row for initial testing
INSERT INTO trie_words (word, frequency)
    VALUES ('example', 1)
ON CONFLICT (word) DO NOTHING;
