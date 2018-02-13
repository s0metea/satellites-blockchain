#!/usr/bin/python
import pandas as pd
import numpy as np
import math
import json as jsn

# Coordinates dimensions
DIM = 3
# Mean Earth radius
EARTH_RADIUS = 6371000.0
# Earth angular speed
EARTH_ROTATION = 7.2921150e-5
# Minimum possible satellite altitude
MIN_SAT_ALT = 1e5


def get_velocity(vel_x, vel_y, vel_z):
    return math.sqrt(vel_x*vel_x + vel_y*vel_y + vel_z*vel_z)


def check_data(nodes, links):
    # Check nodes coordinates dataset
    header = list(nodes.columns.values)
    if (len(header) - 1) % (2 * DIM) != 0:
        return False
    n_stations = (int) (len(header) - 1) / (2 * DIM)
    # Check header for nodes data
    if 'Time(ms)' not in header[0]:
        return False
    expected_subheader = ['Px(m)', 'Py(m)', 'Pz(m)', 'Vx(m/s)', 'Vy(m/s)', 'Vz(m/s)']
    for sta_id in range(1, (int) (n_stations + 1)):
        for j in range(2 * DIM):
            column_header = header[2 * DIM * (sta_id - 1) + j + 1]
            if expected_subheader[j] not in column_header or str(sta_id) not in column_header:
                return False
    # Check links dataset headers
    header = list(links.columns.values)
    # Links are ravelled adjucency matrices for each timestamp

    if len(header) != n_stations ** 2 + 1:
        return False
    if 'Time(ms)' not in header[0]:
        return False
    if not np.array_equal(nodes['Time(ms)'].as_matrix(), links['Time(ms)'].as_matrix()):
        # Timestamps are not synchronized
        return False
    for i in range(int(n_stations)):
        for j in range(int(n_stations)):
            if 'm_{0}_{1}'.format(i + 1, j + 1) not in header[int(n_stations * i + j + 1)]:
                return False
    return True


def generate_trace(timestamp, positions, velocities, links):
    nodes = []
    names_file = open("./data/data.trace", "w+")
    nodeIndex = 0
    for i in range(len(positions)):
        #print("Node {} start position: x={} y={} z={} ".format(i, x, y, z))
        names_file.write('$node_({}) set X_ {}\n'.format(i * 2, positions[i][0][0]))
        names_file.write('$node_({}) set Y_ {}\n'.format(i * 2, positions[i][0][1]))
        names_file.write('$node_({}) set Z_ {}\n'.format(i * 2, positions[i][0][2]))
        #Adding gateways
        names_file.write('$node_({}) set X_ {}\n'.format(i * 2 + 1, positions[i][0][0]))
        names_file.write('$node_({}) set Y_ {}\n'.format(i * 2 + 1, positions[i][0][1]))
        names_file.write('$node_({}) set Z_ {}\n'.format(i * 2 + 1, positions[i][0][2]))

    #Let assume such function that completely describes satellites path. We have data for each 30s of it
    #We want to get position and velocity for every second, so we do linear interpolation
    #f(x0) is a start value and f(x1) the next known
    #f(x) = f(x0) + x1*(f(x1)-f(x0))/30
    #x0 = 0 x1 = 30
    data = []
    # x0 = 0
    # for time in timestamp[:1000]:
    #     pos_x = pos_y = pos_z = vel_x = vel_y = vel_z = 0
    #     for i in range(30):
    #         for satellite in range(len(positions)):
    #             pos_x = ((i * (positions[satellite][x0 + 1][0] - positions[satellite][x0][0])) / 30.0) + positions[satellite][x0][0]
    #             pos_y = ((i * (positions[satellite][x0 + 1][1] - positions[satellite][x0][1])) / 30.0) + positions[satellite][x0][1]
    #             pos_z = ((i * (positions[satellite][x0 + 1][2] - positions[satellite][x0][2])) / 30.0) + positions[satellite][x0][2]
    #             vel_x = ((i * (velocities[satellite][x0 + 1][0] - velocities[satellite][x0][0])) / 30.0) + velocities[satellite][x0][0]
    #             vel_y = ((i * (velocities[satellite][x0 + 1][1] - velocities[satellite][x0][1])) / 30.0) + velocities[satellite][x0][1]
    #             vel_z = ((i * (velocities[satellite][x0 + 1][2] - velocities[satellite][x0][2])) / 30.0) + velocities[satellite][x0][2]
    #             names_file.write('$ns_ at {} "$node_({}) setdest {} {} {} {}"\n'.format(time / 1000 + i,
    #                                                                                       satellite,
    #                                                                                       pos_x,
    #                                                                                       pos_y,
    #                                                                                       pos_z,
    #                                                                                       get_velocity(vel_x, vel_y, vel_z)))
    #             names_file.write('$ns_ at {} "$node_({}) setdest {} {} {} {}"\n'.format(time / 1000 + i,
    #                                                                                     satellite,
    #                                                                                     pos_x,
    #                                                                                     pos_y,
    #                                                                                     pos_z,
    #                                                                                     get_velocity(vel_x, vel_y,
    #                                                                                                  vel_z)))
    #   x0 = x0 + 1

    x = 0
    finalIndex = 100
    if finalIndex > len(timestamp):
        finalIndex = len(timestamp)
    for time in range(finalIndex):
        pos_x = pos_y = pos_z = vel_x = vel_y = vel_z = 0
        for satellite in range(len(positions)):
            pos_x = positions[satellite][x][0]
            pos_y = positions[satellite][x][1]
            pos_z = positions[satellite][x][2]
            vel_x = velocities[satellite][x][0]
            vel_y = velocities[satellite][x][1]
            vel_z = velocities[satellite][x][2]
            names_file.write('$ns_ at {} "$node_({}) setdest {} {} {} {}"\n'.format(time / 1000 + i,
                                                                                    satellite * 2,
                                                                                    pos_x,
                                                                                    pos_y,
                                                                                    pos_z,
                                                                                    get_velocity(vel_x, vel_y, vel_z)))
            #Gateway has the same position and velocity
            names_file.write('$ns_ at {} "$node_({}) setdest {} {} {} {}"\n'.format(time / 1000 + i,
                                                                                    satellite * 2 + 1,
                                                                                    pos_x,
                                                                                    pos_y,
                                                                                    pos_z,
                                                                                    get_velocity(vel_x, vel_y, vel_z)))
        x = x + 1
    names_file.close()


def load_data(nodes_path, links_path):
    nodes = pd.read_csv(nodes_path, sep=';')
    links = pd.read_csv(links_path, sep=';')
    assert check_data(nodes, links)
    header = list(nodes.columns.values)
    n_stations = int(len(header) - 1) / (2 * DIM)
    print('Expected {0} stations.'.format(n_stations))
    print('Expected {0} gateways.'.format(n_stations))

    timestamp = nodes['Time(ms)'].as_matrix()
    nodes = nodes.as_matrix()[:, 1:]  # Strip timestamps
    links = links.as_matrix()[:, 1:]  # Strip timestamps

    positions = {}
    velocities = {}
    for i in range(int(n_stations)):
        positions[i] = nodes[:, 2 * DIM * i:2 * DIM * i + DIM]
        velocities[i] = nodes[:, 2 * DIM * i + DIM:2 * DIM * (i + 1)]

    generate_trace(timestamp, positions, velocities, links)

    return {
        'timestamp': timestamp,
        'positions': positions,
        'velocities': velocities,
        'links': links.reshape(-1, int(n_stations), int(n_stations))
    }


if __name__ == '__main__':
    import os
    import matplotlib.pyplot as plt
    # Data prvoded by the propagator. Note, that header is removed for easier processing.
    # The header describes the set of satellites and ground stations. Results are provided in EME2000 reference frame
    # Note, that all spaces and headers are removed before loading the data
    data_root = 'data'
    data = load_data(os.path.join(data_root, 'nodes.csv'), os.path.join(data_root, 'links.csv'))

    connected_components = []
    n_steps = len(data['timestamp']) - 1
    for i in range(n_steps):
        adjacency = data['links'][i]
        laplacian = np.diag(np.sum(adjacency, axis=0)) - adjacency
        eigen_vals, _ = np.linalg.eig(laplacian)
        connected_components += [np.sum(eigen_vals <= np.finfo(np.float32).eps)]

    print("Links file creation...")
    file = open("./data/links", "w+")
    links = data['links']
    index = 0
    nodeIndex = 0
    for moment_of_time in links[:100]:
        file.write('{}s\n'.format(index * 30))
        for node in moment_of_time:
            for col in node:
                file.write('{} '.format(int(col)))
            for col in node:
                file.write('{} '.format(int(col)))
            file.write('\n')
        index += 1
    file.close()