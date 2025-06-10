import json
from jtop import jtop


with jtop() as jetson:
    if jetson.ok():
        cpu_dump = jetson.cpu
        cpu_stats = {}
        for idx, core in enumerate(cpu_dump['cpu']):
            cpu_stats[f'cpu_{idx}_freq'] = round(core['freq']['cur'] // 1000)  # MHz
            cpu_stats[f'cpu_{idx}_util'] = round(core['user'] + core['system'])  # %

        cpu_stats['cpu_total_util'] = round(cpu_dump['total']['user'] + cpu_dump['total']['system'])  # %

        ram = jetson.memory['RAM']
        memory_stats = {
            'ram_total': round(ram['tot'] // 1024),  # MB
            'ram_used': round(ram['used'] // 1024),  # MB
            'ram_util': round((ram['used'] / ram['tot']) * 100),  # %
        }

        gpu_dump = jetson.gpu
        gpu_stats = {
            'gpu_util': round(gpu_dump['gpu']['status']['load']),  # %
            'gpu_freq': round(gpu_dump['gpu']['freq']['cur'] // 1000),  # MHz
        }

        disk_dump = jetson.disk
        disk_stats = {
            'disk_total': round(disk_dump['total'] * 1024),  # MB
            'disk_used': round(disk_dump['used'] * 1024),  # MB
            'disk_util': round((disk_dump['used'] / disk_dump['total']) * 100),  # %
        }

        temp_dump = jetson.temperature
        temp_stats = {
            'cpu_temp': round(temp_dump['cpu']['temp']),  # °C
            'gpu_temp': round(temp_dump['gpu']['temp']),  # °C
        }

        power_dump = jetson.power
        power_stats = {
            'voltage_draw': round(power_dump['tot']['volt']),  # mV
            'current_draw': round(power_dump['tot']['curr']),  # mA
            'power_draw': round(power_dump['tot']['power']),  # mW
        }

        all_stats = {**cpu_stats, **memory_stats, **gpu_stats, **disk_stats, **temp_stats, **power_stats}
        print(json.dumps(all_stats))

    else:
        print("error")
