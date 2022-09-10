import os
Import("env")

# shutil.copyfile('../.pio/build/esp32cam/firmware.bin', '../../wibee_fw_release')
def copy_release(source, target, env):
    print("RELEASE")

    if os.path.exists("version"):
        FILE = open('version')
        VERSION_NUMBER = FILE.readline()
        print('Build number: {} (waiting for upload before next increment)'.format(str(VERSION_NUMBER)))
    else:
        print('No version file found or incorrect data in it!!')

    cmd = 'echo F | xcopy /y .pio\\build\\esp32cam\\firmware.bin .\\wibee_fw_release\\wibee_esp_firmware_' + VERSION_NUMBER + '.bin'
    print(cmd)
    os.system(cmd)
    
    cmd = 'echo F | xcopy /y .pio\\build\\esp32cam\\firmware.elf .\\wibee_fw_release\\wibee_esp_firmware_' + VERSION_NUMBER + '.elf'
    print(cmd)
    os.system(cmd)
    
    cmd = 'echo F | xcopy /y .pio\\build\\esp32cam\\firmware.bin .\\wibee_fw_release\\firmware.bin'
    print(cmd)
    os.system(cmd)
    
    cmd = 'echo F | xcopy /y .pio\\build\\esp32cam\\firmware.bin .pio\\build\\esp32cam\\wibee_esp_firmware_' + VERSION_NUMBER + '.bin'
    print(cmd)
    os.system(cmd)


env.AddPreAction("upload", copy_release)