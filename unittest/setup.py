from setuptools import setup

setup(
    name='libnitrokey',
    version='0.1',
    py_modules=['libnitrokey'],
    install_requires=[
        'Click',
        'cffi',
    ],
    entry_points='''
        [console_scripts]
        libnitrokey=libnitrokey:cli
    ''',
)