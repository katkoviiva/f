import numpy as np
import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import DataLoader, Dataset
from torchtext.data.utils import get_tokenizer


# ---------- DATASET ----------

class TextDataset(Dataset):
    def __init__(self, text, tokenizer, seq_length=5):
        self.tokenizer = tokenizer
        self.seq_length = seq_length

        self.tokens = tokenizer(text)
        self.data = []
        for i in range(len(self.tokens) - self.seq_length):
            X = self.tokens[i:i + self.seq_length]
            Y = self.tokens[i + self.seq_length]
            self.data.append((X, Y))

    def __len__(self):
        return len(self.data)

    def __getitem__(self, idx):
        X, Y = self.data[idx]
        return X, Y


# ---------- VOCAB / ENCODING ----------

def build_vocab(tokens):
    vocab = sorted(set(tokens))
    word_to_idx = {w: i for i, w in enumerate(vocab)}
    idx_to_word = {i: w for w, i in word_to_idx.items()}
    return vocab, word_to_idx, idx_to_word


def tokens_to_indices(tokens, word_to_idx):
    return [word_to_idx[t] for t in tokens]


# ---------- MODEL ----------

class NeuralNetworkModel(nn.Module):
    def __init__(self, vocab_size, embed_dim, hidden_dim, seq_length):
        super().__init__()
        self.embedding = nn.Embedding(vocab_size, embed_dim)
        self.rnn = nn.GRU(embed_dim, hidden_dim, batch_first=True)
        self.fc = nn.Linear(hidden_dim, vocab_size)
        self.seq_length = seq_length

    def forward(self, x):
        # x: (batch, seq_len)
        emb = self.embedding(x)              # (batch, seq_len, embed_dim)
        out, _ = self.rnn(emb)               # (batch, seq_len, hidden_dim)
        last = out[:, -1, :]                 # (batch, hidden_dim)
        logits = self.fc(last)               # (batch, vocab_size)
        return logits


# ---------- OPTIONAL: NUM_FEED (ASCII) ----------

def num_feed(file_path):
    num_string = ""
    try:
        with open(file_path, "r") as f:
            for line in f:
                line = line.strip()
                if line.isdigit():
                    num_string += line
    except FileNotFoundError:
        print(f"File not found: {file_path}")
        return None
    except Exception as e:
        print(f"An error occurred: {e}")
        return None

    ascii_codes = [num_string[i:i+3] for i in range(0, len(num_string), 3)]
    ascii_chars = [chr(int(code)) for code in ascii_codes if code]
    return "".join(ascii_chars)


# ---------- MAIN EXAMPLE ----------

if __name__ == "__main__":
    # Esimerkkiteksti
    book_text = "hello world this is a tiny example hello world this is another example"

    tokenizer = get_tokenizer("basic_english")
    dataset = TextDataset(book_text, tokenizer, seq_length=5)

    # kerää kaikki X‑tokenit vocabia varten
    all_tokens = []
    for X, _ in dataset:
        all_tokens.extend(X)

    vocab, word_to_idx, idx_to_word = build_vocab(all_tokens)
    vocab_size = len(vocab)

    # indeksöity dataset
    class IndexedTextDataset(Dataset):
        def __init__(self, base_dataset, word_to_idx):
            self.base = base_dataset
            self.word_to_idx = word_to_idx

        def __len__(self):
            return len(self.base)

        def __getitem__(self, idx):
            X_tokens, Y_token = self.base[idx]
            X_idx = torch.tensor(tokens_to_indices(X_tokens, self.word_to_idx), dtype=torch.long)
            Y_idx = torch.tensor(self.word_to_idx[Y_token], dtype=torch.long)
            return X_idx, Y_idx

    indexed_dataset = IndexedTextDataset(dataset, word_to_idx)
    data_loader = DataLoader(indexed_dataset, batch_size=2, shuffle=True)

    embedding_dim = 10
    hidden_dim = 50
    model = NeuralNetworkModel(vocab_size, embedding_dim, hidden_dim, seq_length=5)

    criterion = nn.CrossEntropyLoss()
    optimizer = optim.Adam(model.parameters(), lr=0.001)

    epochs = 50
    for epoch in range(epochs):
        total_loss = 0.0
        for X_batch, Y_batch in data_loader:
            optimizer.zero_grad()
            logits = model(X_batch)
            loss = criterion(logits, Y_batch)
            loss.backward()
            optimizer.step()
            total_loss += loss.item()
        print(f"Epoch {epoch+1}/{epochs}, Loss: {total_loss/len(data_loader):.4f}")

    def predict_next_word(input_sequence_tokens):
        model.eval()
        with torch.no_grad():
            idx_seq = torch.tensor(
                tokens_to_indices(input_sequence_tokens, word_to_idx),
                dtype=torch.long
            ).unsqueeze(0)  # (1, seq_len)
            logits = model(idx_seq)
            _, pred_idx = torch.max(logits, dim=1)
            return idx_to_word[pred_idx.item()]

    test_input = "hello world this is".split()
    predicted = predict_next_word(test_input)
    print(f"Input: {' '.join(test_input)}")
    print(f"Predicted next word: {predicted}")
