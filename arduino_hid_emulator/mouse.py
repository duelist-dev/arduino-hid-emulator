class MouseController:
    """
    Класс для управления виртуальной мышью.
    """

    def __init__(self, arduino_connection):
        self.arduino = arduino_connection

    def move_direct(self, x, y):
        """
        Перемещает мышь на заданное смещение.
        """
        return self.arduino.send_command(f"mouse_move_direct {x},{y}")

    def click(self, button="left"):
        """
        Нажимает и отпускает указанную кнопку мыши.
        """
        return self.arduino.send_command(f"mouse_click {button}")

    def press(self, button="left"):
        """
        Нажимает указанную кнопку мыши.
        """
        return self.arduino.send_command(f"mouse_down {button}")

    def release(self, button="left"):
        """
        Отпускает указанную кнопку мыши.
        """
        return self.arduino.send_command(f"mouse_up {button}")
