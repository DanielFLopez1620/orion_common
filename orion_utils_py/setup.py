from setuptools import find_packages, setup

package_name = 'orion_utils_py'

setup(
    name=package_name,
    version='1.8.2',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='dan1620',
    maintainer_email='dfelipe.lopez@gmail.com',
    description='Common python utilities and emos',
    license='BSD-3-Clause',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'laser_filter = orion_utils_py.laser_filter:main',
            'introducing_orion = orion_utils_py.introducing_orion:main',
            'checking_mov = orion_utils_py.check_mov:main',
            'happy_birthday = orion_utils_py.happy_birthday:main',
            'demo_theater = orion_utils_py.demo_theater:main',
            'emotion_try = orion_utils_py.emotion_try:main',
            'hi_human = orion_utils_py.hi_human:main',
        ],
    },
)
