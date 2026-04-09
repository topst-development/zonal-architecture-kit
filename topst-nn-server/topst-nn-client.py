#!/usr/bin/env python3

import argparse
import colorsys
import json
import select
import socket
from io import StringIO

import cv2


def parse_cmdline():
    parser = argparse.ArgumentParser(description='TOPST NN Client App')
    parser.add_argument('ip', type=str, help='IP address')

    # optionals
    parser.add_argument('-i',
                        '--index',
                        type=int,
                        default=0,
                        help='Camera Device ID (default: 0)')
    parser.add_argument('-p',
                        '--port',
                        type=int,
                        default=4444,
                        help='Port number (default: 4444)')
    parser.add_argument('-v',
                        '--verbose',
                        action='store_true',
                        default=False,
                        help='Enable verbose mode')
    parser.add_argument('-l',
                        '--label',
                        type=str,
                        default=None,
                        help='Path to label list file')
    parser.add_argument('-W',
                        '--width',
                        type=int,
                        default=640,
                        help='Camera width (default: 640)')
    parser.add_argument('-H',
                        '--height',
                        type=int,
                        default=480,
                        help='Camera height (default: 480)')
    parser.add_argument('-c',
                        '--colors',
                        type=int,
                        default=1000,
                        help='Max label colors (default: 1000)')

    return parser.parse_args()


def get_distinct_colors(n):
    hue_partition = 1. / (n + 1)

    def hsv2rgb(h, s, v):
        (r, g, b) = colorsys.hsv_to_rgb(h, s, v)
        return (int(255 * r), int(255 * g), int(255 * b))

    return tuple(hsv2rgb(hue_partition * v, 1., 1.) for v in range(n))


def receive_data(sock, buffer_size=4096, timeout=1.):
    data_buffer = StringIO()

    while True:
        ready, *_ = select.select([sock], [], [], timeout)
        if sock not in ready:
            continue

        data = sock.recv(buffer_size)
        if not data:
            break

        data_buffer.write(data.decode('utf-8'))
        data_buffer.seek(0)

        while True:
            try:
                json_data = json.load(data_buffer)
                remaining_data = data_buffer.read()
                data_buffer = StringIO(remaining_data)
                yield json_data
            except json.JSONDecodeError:
                data_buffer.seek(0, 2)
                break


def draw_result_classifier(frame, result, colors, labels, *_):
    class_id = int(result[0])
    name = f'{labels[class_id]}' if labels else f'#{class_id}'

    cv2.putText(frame, name, (20, 20), cv2.FONT_HERSHEY_SIMPLEX, 0.5,
                colors[class_id % len(colors)], 2)


def draw_result_detector(frame, result, colors, labels, info):
    rw, rh = info['ratio']

    for (class_id, score, x_min, y_min, x_max, y_max) in result:
        name = f'{labels[class_id]}' if labels else f'#{class_id}'
        color = colors[class_id % len(colors)]

        x_min = int(x_min * rw)
        y_min = int(y_min * rh)
        x_max = int(x_max * rw)
        y_max = int(y_max * rh)

        cv2.rectangle(frame, (x_min, y_min), (x_max, y_max), color, 2)
        cv2.putText(frame, name, (x_min + 20, y_min + 20),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 2)


def main(args):

    # load labels info
    if args.label is not None:
        with open(args.label, 'rt') as f:
            labels = tuple(l.strip() for l in f.readlines())
        colors = get_distinct_colors(max(args.colors, len(labels)))
    else:
        labels = None
        colors = get_distinct_colors(args.colors)

    # prepare camera
    cam = cv2.VideoCapture(args.index)
    cam.set(cv2.CAP_PROP_FRAME_WIDTH, args.width)
    cam.set(cv2.CAP_PROP_FRAME_HEIGHT, args.height)
    frame_width = int(cam.get(cv2.CAP_PROP_FRAME_WIDTH))
    frame_height = int(cam.get(cv2.CAP_PROP_FRAME_HEIGHT))
    print('[!] Camera Size:', f'{frame_width}x{frame_height}')

    # connect to the topst-nn-server
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((args.ip, args.port))

    # get model info
    reader = receive_data(sock)
    info = next(reader)
    print('[!] Model Type:', info['type'])
    print('[!] Input Size:', f'{info["input_width"]}x{info["input_height"]}')

    info['frame_width'] = frame_width
    info['frame_height'] = frame_height
    info['ratio'] = (frame_width / info["input_width"],
                     frame_height / info["input_height"])

    dsize = (info["input_width"], info["input_height"])
    if info['type'] == 'classifier':
        draw_result = draw_result_classifier
    else:
        draw_result = draw_result_detector

    while 1:
        ret, frame = cam.read()
        if not ret:
            break

        # preprocessing
        reshaped = cv2.resize(frame, dsize=dsize, interpolation=cv2.INTER_AREA)
        reshaped = cv2.cvtColor(reshaped, cv2.COLOR_BGR2RGB)
        data = reshaped.astype('int8').tobytes()

        # send to NPU
        sock.send(data)

        # draw result
        result = next(reader)
        draw_result(frame, result, colors, labels, info)

        cv2.imshow('Result', frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    cam.release()
    sock.close()

    cv2.destroyAllWindows()


if __name__ == '__main__':
    main(parse_cmdline())
