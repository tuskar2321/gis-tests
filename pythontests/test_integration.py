import hashlib
import pymapapi

GISSERVER_HOST = "gisserver"
GISSERVER_PORT = 2047
GISSERVER_USER = "User1"
GISSERVER_PASS = "1"

def test_connects_to_gisserver():
    pymapapi.mapMessageEnable(0)

    connection = pymapapi.mapOpenConnectUn(GISSERVER_HOST, GISSERVER_PORT)
    assert connection != 0

    md5hash = hashlib.md5()
    md5hash.update(GISSERVER_PASS.encode("utf-8"))
    hashed_password = md5hash.hexdigest().upper()

    is_registered = pymapapi.mapRegisterUserEx(connection, GISSERVER_USER, hashed_password)
    assert is_registered != 0

    map_mode = 0
    map_path = "HOST#" + GISSERVER_HOST + "#" + str(GISSERVER_PORT) + "#ALIAS#Noginsk"
    map_handle = pymapapi.mapOpenAnyData(map_path, map_mode)
    assert map_handle != 0
    print(map_handle)

#   print("adjust_data")
#    pymapapi.mapAdjustData(map_handle)
#    assert 1 == 0

    pymapapi.mapCloseData(map_handle)
    pymapapi.mapCloseConnect(connection)
