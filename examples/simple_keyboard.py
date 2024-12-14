from arduino_hid_emulator.arduino import ArduinoConnection
from arduino_hid_emulator.keyboard import KeyboardController

arduino = ArduinoConnection()
keyboard = KeyboardController(arduino)

keyboard.press_key("a")  # Нажимает клавишу "a"
keyboard.release_key("a")  # Отпускает клавишу "a"
keyboard.type_key("b")  # Нажимает и отпускает клавишу "b"
keyboard.key_combo("ctrl+alt+del")  # Выполняет комбинацию
arduino.close()
