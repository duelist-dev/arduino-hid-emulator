#include <Keyboard.h>
#include <Mouse.h>
#include <math.h>

// Интервалы для выполнения действий с клавиатурой и мышью
#define KEY_INTERVAL_MIN 150  // Минимальный интервал (мс)
#define KEY_INTERVAL_MAX 300  // Максимальный интервал (мс)
#define MOUSE_MOVE_DURATION_MIN 500  // Минимальная продолжительность движения мыши (мс)
#define MOUSE_MOVE_DURATION_MAX 1500 // Максимальная продолжительность движения мыши (мс)

void setup() {
    Keyboard.begin();   // Инициализация HID клавиатуры
    Mouse.begin();      // Инициализация HID мыши
    Serial.begin(9600); // Устанавливаем соединение для взаимодействия с Python
}

void loop() {
    // Проверяем, есть ли входящие команды через последовательный порт
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n'); // Читаем команду до символа новой строки
        bool result = processCommand(command);        // Обрабатываем команду
        Serial.println(result ? "True" : "False");    // Возвращаем результат выполнения команды
    }
}

// Обработка команды прямолинейного движения мыши
bool handleMouseMoveDirect(String params) {
    int sep = params.indexOf(','); // Определяем позицию разделителя (запятая)
    if (sep == -1) return false;   // Если запятая отсутствует, возвращаем false

    // Парсим координаты X и Y из строки
    int targetX = params.substring(0, sep).toInt();
    int targetY = params.substring(sep + 1).toInt();

    // Перемещаем мышь на указанные координаты без задержек
    Mouse.move(targetX, targetY);
    return true;
}

// Обработка входящих команд
bool processCommand(String command) {
    // Определяем тип команды и вызываем соответствующую функцию
    if (command.startsWith("mouse_move_direct ")) {
        return handleMouseMoveDirect(command.substring(18));
    } else if (command.startsWith("key_down ")) {
        char key = parseKey(command.substring(9));
        delay(random(KEY_INTERVAL_MIN, KEY_INTERVAL_MAX)); // Задержка перед нажатием
        Keyboard.press(key);
        return true;
    } else if (command.startsWith("key_up ")) {
        char key = parseKey(command.substring(7));
        delay(random(KEY_INTERVAL_MIN, KEY_INTERVAL_MAX)); // Задержка перед отпусканием
        Keyboard.release(key);
        return true;
    } else if (command.startsWith("key_press ")) {
        char key = parseKey(command.substring(10));
        delay(random(KEY_INTERVAL_MIN, KEY_INTERVAL_MAX)); // Задержка перед нажатием
        Keyboard.press(key);
        delay(random(KEY_INTERVAL_MIN, KEY_INTERVAL_MAX)); // Задержка между нажатием и отпусканием
        Keyboard.release(key);
        return true;
    } else if (command.startsWith("key_combo ")) {
        return handleKeyCombo(command.substring(10));
    } else if (command.startsWith("mouse_move ")) {
        return handleMouseMove(command.substring(11));
    } else if (command.startsWith("mouse_down ")) {
        return handleMousePress(command.substring(11), true);
    } else if (command.startsWith("mouse_up ")) {
        return handleMousePress(command.substring(9), false);
    } else if (command.startsWith("mouse_click ")) {
        return handleMouseClick(command.substring(12));
    }
    return false; // Если команда не распознана, возвращаем false
}

// Преобразование строки в символ клавиши
char parseKey(String keyStr) {
    keyStr.trim(); // Удаляем пробелы в начале и конце строки
    // Проверяем на совпадение с известными клавишами
    if (keyStr == "ctrl") return KEY_LEFT_CTRL;
    if (keyStr == "shift") return KEY_LEFT_SHIFT;
    if (keyStr == "alt") return KEY_LEFT_ALT;
    if (keyStr == "tab") return KEY_TAB;
    if (keyStr == "enter") return KEY_RETURN;
    if (keyStr == "space") return ' ';
    return keyStr[0]; // Если символ не распознан, возвращаем первый символ строки
}

// Обработка комбинации клавиш, например "ctrl+tab"
bool handleKeyCombo(String combo) {
    int idx = combo.indexOf('+'); // Определяем разделитель '+'
    if (idx == -1) return false;  // Если '+' отсутствует, возвращаем false

    // Парсим модификатор и основную клавишу
    char modKey = parseKey(combo.substring(0, idx));
    char mainKey = parseKey(combo.substring(idx + 1));

    // Нажимаем модификатор, затем основную клавишу
    delay(random(KEY_INTERVAL_MIN, KEY_INTERVAL_MAX));
    Keyboard.press(modKey);
    delay(random(KEY_INTERVAL_MIN, KEY_INTERVAL_MAX));
    Keyboard.press(mainKey);

    // Отпускаем клавиши в обратном порядке
    delay(random(KEY_INTERVAL_MIN, KEY_INTERVAL_MAX));
    Keyboard.release(mainKey);
    delay(random(KEY_INTERVAL_MIN, KEY_INTERVAL_MAX));
    Keyboard.release(modKey);

    return true;
}

// Плавное движение мыши по кривой Безье
bool handleMouseMove(String params) {
    // Разделяем параметры команды
    int sep1 = params.indexOf(',');
    if (sep1 == -1) return false;

    int sep2 = params.indexOf(',', sep1 + 1);
    if (sep2 == -1) return false;

    int sep3 = params.indexOf(',', sep2 + 1);
    if (sep3 == -1) return false;

    // Парсим координаты и коэффициенты
    int targetX = params.substring(0, sep1).toInt();
    int targetY = params.substring(sep1 + 1, sep2).toInt();
    float factorX = params.substring(sep2 + 1, sep3).toFloat();
    float factorY = params.substring(sep3 + 1).toFloat();

    // Применяем коэффициенты к целевым координатам
    targetX = round(targetX * factorX);
    targetY = round(targetY * factorY);

    // Проверка: если конечная цель равна (0,0), предотвращаем движение
    if (targetX == 0 && targetY == 0) {
        return false;
    }

    // Длительность движения
    unsigned long duration = random(MOUSE_MOVE_DURATION_MIN, MOUSE_MOVE_DURATION_MAX);
    unsigned long startTime = millis();

    int steps = 100; // Количество шагов для плавности

    // Текущие координаты
    int currentX = 0, currentY = 0;

    // Вычисляем параметры кривой Безье
    float dx = targetX - currentX;
    float dy = targetY - currentY;
    float distance = sqrt(dx * dx + dy * dy);
    float offset = distance * 0.2;

    float normX = -dy / distance;
    float normY = dx / distance;

    int controlX1 = targetX / 3 + normX * offset;
    int controlY1 = targetY / 3 + normY * offset;
    int controlX2 = 2 * targetX / 3 - normX * offset;
    int controlY2 = 2 * targetY / 3 - normY * offset;

    // Движение по Безье
    for (int i = 1; i <= steps; i++) {
        float t = (float)i / steps;
        int bezierX = round((1 - t) * (1 - t) * (1 - t) * 0 +
                            3 * (1 - t) * (1 - t) * t * controlX1 +
                            3 * (1 - t) * t * t * controlX2 +
                            t * t * t * targetX);
        int bezierY = round((1 - t) * (1 - t) * (1 - t) * 0 +
                            3 * (1 - t) * (1 - t) * t * controlY1 +
                            3 * (1 - t) * t * t * controlY2 +
                            t * t * t * targetY);

        int moveX = bezierX - currentX;
        int moveY = bezierY - currentY;

        currentX += moveX;
        currentY += moveY;

        if ((moveX > 0 && currentX > targetX) || (moveX < 0 && currentX < targetX)) {
            currentX = targetX;
            moveX = 0;
        }
        if ((moveY > 0 && currentY > targetY) || (moveY < 0 && currentY < targetY)) {
            currentY = targetY;
            moveY = 0;
        }

        Mouse.move(moveX, moveY);
        delay(duration / steps);
    }

    // Финальная коррекция
    int finalX = targetX - currentX;
    int finalY = targetY - currentY;
    int correctionSteps = 20;
    for (int i = 0; i < correctionSteps; i++) {
        int stepX = finalX / correctionSteps;
        int stepY = finalY / correctionSteps;
        Mouse.move(stepX, stepY);
        finalX -= stepX;
        finalY -= stepY;
        delay(200 / correctionSteps);
    }

    return true;
}

// Нажатие или отпускание кнопок мыши
bool handleMousePress(String button, bool isDown) {
    delay(random(KEY_INTERVAL_MIN, KEY_INTERVAL_MAX));
    if (button == "left") {
        if (isDown) Mouse.press(MOUSE_LEFT);
        else Mouse.release(MOUSE_LEFT);
    } else if (button == "right") {
        if (isDown) Mouse.press(MOUSE_RIGHT);
        else Mouse.release(MOUSE_RIGHT);
    }
    return true;
}

// Имитация клика мыши
bool handleMouseClick(String button) {
    return handleMousePress(button, true) && handleMousePress(button, false);
}
