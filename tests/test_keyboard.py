from unittest.mock import MagicMock
from arduino_hid_emulator.keyboard import KeyboardController


def test_press_key():
    mock_arduino = MagicMock()
    keyboard = KeyboardController(mock_arduino)

    keyboard.press_key("a")
    mock_arduino.send_command.assert_called_with("key_down a")
