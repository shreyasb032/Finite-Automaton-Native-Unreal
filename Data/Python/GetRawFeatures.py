import pandas as pd
import os

data_directory = 'PandasData'
output_directory = '..'

files = os.listdir(data_directory)

for file in files:
    filepath = os.path.join(data_directory, file)
    data = pd.read_csv(filepath, usecols=["GazeDirection_X", "GazeDirection_Y",	"GazeDirection_Z",
                                          "AGV_X", "AGV_Y",	"User_X", "User_Y",	"AGV_name"])

    out_filepath = os.path.join(output_directory, file)
    data.to_csv(out_filepath, index=False)
