import json
import sys
import random

def get_affiliate_links(state, products_path):
    try:
        with open(products_path, 'r') as f:
            data = json.load(f)
        
        links = []
        # Get general links
        links.extend(data.get("general", []))
        # Get state-specific links
        links.extend(data.get(state, []))
        
        # Pick 2-3 random links total
        random.shuffle(links)
        return links[:3]
    except Exception as e:
        return ["Discover more in our Store: [LINK]"]

if __name__ == "__main__":
    state = sys.argv[1] if len(sys.argv) > 1 else "general"
    path = sys.argv[2] if len(sys.argv) > 2 else "products.json"
    
    links = get_affiliate_links(state, path)
    for link in links:
        print(link)
