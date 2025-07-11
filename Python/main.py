import pandas as pd
import numpy as np

df = pd.read_pickle("PID001_NSL.pkl")
for drop_column in ['Confidence', 'Timestamp', 'TimestampID',
                    'DatapointID', 'PID', 'SCN', 'U_X', 'U_Y', 'U_Z',
                    'AGV_Z', 'User_Z', 'GazeOrigin_Z', 'User_Pitch', 'User_Yaw', 'User_Roll',
                    'EyeTarget', "AGV_name"]:
    df = df.drop(columns=[drop_column], errors='ignore')

df = df.apply(pd.to_numeric)
df = df.astype(np.float32)
print(df.columns)
df.to_csv('PID001_NSL.csv', index=False)
