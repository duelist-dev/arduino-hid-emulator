from setuptools import setup, find_packages

setup(
    name="arduino-hid-emulator",
    version="0.1.0",
    author="Andrey Mishchenko",
    author_email="msav@msav.ru",
    description="Python module for controlling an Arduino-based HID emulator for keyboard and mouse.",
    long_description=open("README.md").read(),
    long_description_content_type="text/markdown",
    url="https://github.com/mvandrew/arduino-hid-emulator",
    packages=find_packages(exclude=["arduino", "arduino.*"]),
    package_dir={"": "."},  # Указывает корневую директорию проекта
    install_requires=[
        "pyserial>=3.5",
        "pyautogui>=0.9.53",
    ],
    extras_require={
        "dev": ["pytest>=7.0", "mock>=5.0"],
    },
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: GNU General Public License v3 (GPLv3)",
        "Operating System :: OS Independent",
    ],
    python_requires=">=3.6",
)
