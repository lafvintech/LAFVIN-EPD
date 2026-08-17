from setuptools import find_packages, setup


setup(
    name="lafvin-epd",
    version="0.1.0",
    description="LAFVIN 2.9-inch four-color Raspberry Pi e-paper driver",
    package_dir={"": "lib"},
    packages=find_packages("lib"),
    python_requires=">=3.8",
    install_requires=["Pillow>=8.0"],
)
