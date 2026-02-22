import xml.etree.ElementTree as ET

tree = ET.parse('system_architecture.xml')
root = tree.getroot()

id_map = {}
count = 2

# Map IDs to integers
for cell in root.iter('mxCell'):
    old_id = cell.get('id')
    if old_id and old_id not in ['0', '1']:
        new_id = str(count)
        id_map[old_id] = new_id
        count += 1

# Update references
for cell in root.iter('mxCell'):
    if cell.get('id') in id_map:
        cell.set('id', id_map[cell.get('id')])
    
    parent = cell.get('parent')
    if parent in id_map:
        cell.set('parent', id_map[parent])
        
    source = cell.get('source')
    if source in id_map:
        cell.set('source', id_map[source])
        
    target = cell.get('target')
    if target in id_map:
        cell.set('target', id_map[target])

tree.write('system_architecture_fixed.xml', encoding='utf-8')