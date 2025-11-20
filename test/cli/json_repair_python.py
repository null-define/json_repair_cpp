import json_repair
import sys
import argparse
import json


# read file from command line
path=sys.argv[1]
with open(path) as json_file:
    json_string = json_file.read()
    decoded_object = json_repair.loads(json_string)
    print(json.dumps(decoded_object, indent=4))