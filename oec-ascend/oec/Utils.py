from datetime import timedelta

def elapsed_time_str(delta:timedelta):

    hours = delta.seconds // 3600
    minutes = (delta.seconds // 60) % 60
    seconds = round(delta.seconds % 60 + delta.microseconds // 10000 * 0.01,1)
    x = [delta.days,hours,minutes,seconds]
    y = ['d','h','m','s']
    for i in range(len(x)):
        if x[i] > 0 or i == len(x) - 1:
            x = x[i:]
            y = y[i:]
            break
            
    result = ""
    for i in range(len(x)):
        result += f"{x[i]}{y[i]}"
    return result