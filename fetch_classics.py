import sys
import urllib.request
import re

# Curated List of Project Gutenberg IDs for "High Authority Wisdom"
WISDOM_LIBRARY = {
    "Meditations (Marcus Aurelius)": "68",
    "Taciturnity (Epictetus)": "14588",
    "Tao Te Ching (Lao Tzu)": "216",
    "The Prophet (Kahlil Gibran)": "58585",
    "Zen Flesh Zen Bones": "4351" # Example (Check copyright first)
}

def clean_text(text):
    # Remove Project Gutenberg header/footer and non-printable characters
    start_match = re.search(r"\*\*\* START OF (?:THE|THIS) PROJECT GUTENBERG EBOOK.*?\*\*\*", text, re.IGNORECASE)
    end_match = re.search(r"\*\*\* END OF (?:THE|THIS) PROJECT GUTENBERG EBOOK.*?\*\*\*", text, re.IGNORECASE)
    
    if start_match:
        text = text[start_match.end():]
    if end_match:
        text = text[:end_match.start()]
    
    # Clean up whitespace and newlines
    text = re.sub(r'\n+', '\n', text)
    text = text.strip()
    return text

def fetch_book(book_id):
    url = f"https://www.gutenberg.org/cache/epub/{book_id}/pg{book_id}.txt"
    try:
        with urllib.request.urlopen(url) as response:
            text = response.read().decode('utf-8')
            return clean_text(text)
    except Exception as e:
        return f"Error fetching book: {e}"

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: fetch_classics.py <id>")
        sys.exit(1)
    
    book_id = sys.argv[1]
    title = sys.argv[2] if len(sys.argv) > 2 else "Untitled"
    
    print(f"Fetching {title}...")
    book_text = fetch_book(book_id)
    
    # Save a snippet (First 5000 chars) for high-impact summaries/meditations
    with open("wisdom_text.txt", "w") as f:
        f.write(book_text[:10000]) # Limit to 10k chars for manageable demo speed
    
    print("Wisdom fetched and cleaned.")
