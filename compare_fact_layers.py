def extract_atom_content(entry):
    """Extract the actual atom content, removing list formatting and cleaning the string."""
    # Remove list characters and quotes
    entry = entry.replace('[', '').replace(']', '').replace("'", '').replace('"', '')
    
    # Skip if NegatedAtom
    if 'NegatedAtom' in entry:
        return None
        
    # Remove 'Atom ' prefix if it exists
    if 'Atom ' in entry:
        entry = entry.replace('Atom ', '')
    
    # Remove all spaces and commas
    entry = ''.join(entry.split())
    entry = entry.replace(',', '')
    
    if '<noneofthose>' in entry:
        entry = entry.replace('<noneofthose>', '')

    # Check if it's a valid atom (has parentheses)
    if '(' in entry and ')' in entry:
        return entry
    return None

def parse_file(filename):
    """Parse a file and extract all non-negated atoms."""
    atoms = set()
    with open(filename, 'r') as f:
        content = f.read().strip()
        
        # Split by comma followed by space to separate list items
        content = content.replace("'], ['", "', '")
        entries = content.split("', '")
        if len(entries) == 1:  # If no list items found, try regular comma split
            entries = content.split(',')
        
        for entry in entries:
            atom = extract_atom_content(entry)
            if atom:
                atoms.add(atom)
    
    return atoms

def parse_file2(filename):
    """Parse a file and extract all non-negated atoms."""
    atoms = set()
    with open(filename, 'r') as f:
        content = f.read().strip()
        
        # Split by comma followed by space to separate list items
        entries = content.split(" ")
        if len(entries) == 1:  # If no list items found, try regular comma split
            entries = content.split(',')
        
        for entry in entries:
            atom = extract_atom_content(entry)
            if atom:
                atoms.add(atom)
    
    return atoms

def compare_atoms(file1, file2):
    """Compare atoms between two files and report differences."""
    atoms1 = parse_file(file1)
    atoms2 = parse_file2(file2)
    
    # Find atoms unique to each file
    only_in_file1 = atoms1 - atoms2
    only_in_file2 = atoms2 - atoms1
    
    print(f"Atoms found only in {file1}:")
    for atom in sorted(only_in_file1):
        print(f"  {atom}")
    
    print(f"\nAtoms found only in {file2}:")
    for atom in sorted(only_in_file2):
        print(f"  {atom}")
    
    print(f"\nTotal atoms in {file1}: {len(atoms1)}")
    print(f"Total atoms in {file2}: {len(atoms2)}")
    print(f"Atoms in both files: {len(atoms1 & atoms2)}")

# Example usage and test
if __name__ == "__main__":
    compare_atoms("fd.txt", "gen.txt")