import serial
import serial.tools.list_ports
import time


class ArduinoConnection:
    """
    Класс для управления подключением к Arduino.
    """

    def __init__(self, baud_rate=9600):
        self.port = self._find_arduino_port()
        if not self.port:
            raise RuntimeError("Arduino не найден. Проверьте подключение.")
        self.connection = serial.Serial(self.port, baud_rate)
        time.sleep(2)  # Даем время Arduino на перезапуск

    @staticmethod
    def _find_arduino_port():
        """
        Определяет порт Arduino.
        """
        ports = serial.tools.list_ports.comports()
        for port in ports:
            if "Arduino" in port.description or "CH340" in port.description:
                return port.device
        return None

    def send_command(self, command):
        """
        Отправляет команду на Arduino и возвращает ответ.
        """
        if not self.connection.is_open:
            raise RuntimeError("Подключение к Arduino не активно.")
        self.connection.write((command + "\n").encode())
        time.sleep(0.05)  # Небольшая задержка для обработки команды Arduino
        return self.connection.readline().decode().strip()

    def close(self):
        """
        Закрывает соединение с Arduino.
        """
        if self.connection.is_open:
            self.connection.close()
