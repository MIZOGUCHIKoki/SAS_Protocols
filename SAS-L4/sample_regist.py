import ctypes

lib = ctypes.cdll.LoadLibrary('./libSASLotp.so')

class sIFInfo(ctypes.Structure):
    _fields_ = [("Alp", ctypes.c_ulonglong),
                ("Bet", ctypes.c_ulonglong),
                ("Gam", ctypes.c_ulonglong)]

class sCLInfo(ctypes.Structure):
    _fields_ = [("A", ctypes.c_ulonglong),
                ("M", ctypes.c_ulonglong),
                ("LastAlp", ctypes.c_ulonglong)]
    
class sSVInfo(ctypes.Structure):
    _fields_ = [("ID", ctypes.c_ulonglong),
                ("A", ctypes.c_ulonglong),
                ("M", ctypes.c_ulonglong),
                ("B", ctypes.c_ulonglong),
                ("N", ctypes.c_ulonglong),
                ("Alp", ctypes.c_ulonglong),
                ("Bet", ctypes.c_ulonglong),]
# 関数の引数の型を指定する
#lib.example_function.argtypes = [ctypes.c_int, ctypes.c_double]

# 構造体のポインタを受け取る関数を定義
# void SVgenReg(ullong id, sSVInfo *svp);
SVgenReg = lib.SVgenReg
SVgenReg.argtypes = [ctypes.c_ulonglong, ctypes.POINTER(sSVInfo)]
SVgenReg.restype = None

# ullong str2ull(char *str);
str2ull = lib.str2ull
str2ull.argtypes = [ctypes.c_char_p]
str2ull.restype = ctypes.c_ulonglong


# 関数の戻り値の型を指定する
sif = sIFInfo()
svif = sSVInfo()
clif = sCLInfo()
# 関数を呼び出す
print("-----登録フェーズ-----\n")
lib.SVinitSASL()
SVgenReg(str2ull(b'user0001'),svif)
print("A:",format(svif.A,'x'))
print("M:",format(svif.M,'x'))
clif.A = svif.A
clif.M = svif.M
clif.B = 0
clif.N = 0
temp = [0, 0]

# ファイルへの書き込み
with open('SVinfo_py.txt', 'w') as f:
    f.write(str(svif.ID) + '\n')
    f.write(str(svif.A) + '\n')
    f.write(str(svif.M) + '\n')
    f.write(str(svif.B) + '\n')
    f.write(str(svif.N) + '\n')

# ファイルへの書き込み
with open('CLinfo_py.txt', 'w') as f:
    f.write(str(clif.A) + '\n')
    f.write(str(clif.M) + '\n')
    f.write(str(clif.LastAlp) + '\n')

print("\n登録完了しました!\n用途に合わせて「CLinfo_py.txt」をクライアントに渡してください")