class KeyboardController:
    """
    Класс для управления виртуальной клавиатурой.
    """

    def __init__(self, arduino_connection):
        self.arduino = arduino_connection

    def press_key(self, key):
        """
        Нажимает клавишу.
        """
        return self.arduino.send_command(f"key_down {key}")

    def release_key(self, key):
        """
        Отпускает клавишу.
        """
        return self.arduino.send_command(f"key_up {key}")

    def type_key(self, key):
        """
        Нажимает и отпускает клавишу.
        """
        return self.arduino.send_command(f"key_press {key}")

    def key_combo(self, combo):
        """
        Выполняет комбинацию клавиш, например, ctrl+alt+delete.
        """
        return self.arduino.send_command(f"key_combo {combo}")
