import pyautogui
import time


class MouseController:
    """
    Класс для управления виртуальной мышью.
    """

    def __init__(self, arduino_connection, default_factor_x=1/3, default_factor_y=1/3):
        self.arduino = arduino_connection
        self.factor_x = default_factor_x
        self.factor_y = default_factor_y

    def move_direct(self, x, y):
        """
        Перемещает мышь на заданное смещение с учётом коэффициентов.
        """
        adjusted_x = round(x * self.factor_x)
        adjusted_y = round(y * self.factor_y)
        return self.arduino.send_command(f"mouse_move_direct {adjusted_x},{adjusted_y}")

    def smooth_move(self, target_x, target_y, duration=1.0, steps=100):
        """
        Плавно перемещает курсор на заданные координаты за указанное время.

        :param target_x: Смещение по X.
        :param target_y: Смещение по Y.
        :param duration: Время перемещения (в секундах).
        :param steps: Количество шагов.
        """
        # Вычисляем шаг времени
        step_delay = duration / steps

        # Начальное положение
        current_x, current_y = 0, 0

        # Вычисляем инкременты
        increment_x = target_x / steps
        increment_y = target_y / steps

        # Плавное перемещение
        for i in range(steps):
            move_x = round(increment_x * (i + 1)) - current_x
            move_y = round(increment_y * (i + 1)) - current_y

            self.move_direct(move_x, move_y)

            # Обновляем текущее положение
            current_x += move_x
            current_y += move_y

            # Задержка между шагами
            time.sleep(step_delay)

        # Финальная коррекция, чтобы гарантировать точность
        final_x = target_x - current_x
        final_y = target_y - current_y
        if final_x != 0 or final_y != 0:
            self.move_direct(final_x, final_y)

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

    def calibrate(self):
        """
        Калибрует коэффициенты компенсации перемещения указателя.
        """
        print("Начинается калибровка...")

        # Получаем размеры экрана
        screen_width, screen_height = pyautogui.size()
        print(f"Размер экрана: {screen_width}x{screen_height}")

        # Устанавливаем начальные коэффициенты
        self.factor_x, self.factor_y = 1 / 3, 1 / 3

        for attempt in range(3):  # Повторяем 3 раза для уточнения коэффициентов
            print(f"Попытка {attempt + 1} из 3")

            # 1. Перемещаем курсор в центр экрана
            center_x, center_y = screen_width // 2, screen_height // 2
            pyautogui.moveTo(center_x, center_y)
            time.sleep(0.1)  # Даем курсору переместиться
            reference_x, reference_y = pyautogui.position()
            print(f"Курсор установлен в центр экрана: {reference_x}, {reference_y}")

            # 2. Перемещаем курсор на 50 пикселей по X и Y средствами Arduino
            self.move_direct(50, 50)
            time.sleep(0.2)  # Даем время для обработки движения Arduino

            # 3. Фиксируем новые координаты курсора
            new_x, new_y = pyautogui.position()
            print(f"Новое положение курсора: {new_x}, {new_y}")

            # 4. Вычисляем фактическое смещение
            actual_move_x = new_x - reference_x
            actual_move_y = new_y - reference_y

            if actual_move_x == 0 or actual_move_y == 0:
                raise RuntimeError("Не удалось зафиксировать движение курсора. Проверьте соединение.")

            # 5. Обновляем коэффициенты поправки
            self.factor_x *= 50 / actual_move_x
            self.factor_y *= 50 / actual_move_y

            print(f"Обновлённые коэффициенты: factor_x = {self.factor_x}, factor_y = {self.factor_y}")

            # Проверяем точность перемещения
            error_x = abs(actual_move_x * self.factor_x - 50)
            error_y = abs(actual_move_y * self.factor_y - 50)

            if error_x <= 1 and error_y <= 1:
                print("Точность достигнута!")
                break
            else:
                print(f"Текущая ошибка: error_x = {error_x}, error_y = {error_y}")

        print(f"Финальные коэффициенты: factor_x = {self.factor_x}, factor_y = {self.factor_y}")
