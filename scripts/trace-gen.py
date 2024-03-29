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
    return math.sqrt(vel_x * vel_x + vel_y * vel_y + vel_z * vel_z)


def check_data(nodes, links):
    # Check nodes coordinates dataset
    header = list(nodes.columns.values)
    if (len(header) - 1) % (2 * DIM) != 0:
        return False
    n_stations = (int)(len(header) - 1) / (2 * DIM)
    # Check header for nodes data
    if 'Time(ms)' not in header[0]:
        return False
    expected_subheader = ['Px(m)', 'Py(m)', 'Pz(m)', 'Vx(m/s)', 'Vy(m/s)', 'Vz(m/s)']
    for sta_id in range(1, (int)(n_stations + 1)):
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


# Let assume such function that completely describes satellites path. We have data for each 30s of it
# We want to get position and velocity for every second, so we do linear interpolation
# f(x0) is a start value and f(x1) the next known
# f(x) = f(x0) + x1*(f(x1)-f(x0))/30
# x0 = 0 x1 = 30
def generate_trace_with_interpolation(timestamp, positions, velocities, links):
    names_file = open("./data/data.trace", "w+")
    x0 = 0
    for time in timestamp[:1000]:
        for i in range(30):
            for satellite in range(len(positions)):
                pos_x = ((i * (positions[satellite][x0 + 1][0] - positions[satellite][x0][0])) / 30.0) + \
                        positions[satellite][x0][0]
                pos_y = ((i * (positions[satellite][x0 + 1][1] - positions[satellite][x0][1])) / 30.0) + \
                        positions[satellite][x0][1]
                pos_z = ((i * (positions[satellite][x0 + 1][2] - positions[satellite][x0][2])) / 30.0) + \
                        positions[satellite][x0][2]
                vel_x = ((i * (velocities[satellite][x0 + 1][0] - velocities[satellite][x0][0])) / 30.0) + \
                        velocities[satellite][x0][0]
                vel_y = ((i * (velocities[satellite][x0 + 1][1] - velocities[satellite][x0][1])) / 30.0) + \
                        velocities[satellite][x0][1]
                vel_z = ((i * (velocities[satellite][x0 + 1][2] - velocities[satellite][x0][2])) / 30.0) + \
                        velocities[satellite][x0][2]
                velocity = get_velocity(vel_x, vel_y, vel_z)
                names_file.write(
                    '$ns_ at {} "$node_({}) setdest {} {} {} {}"\n'.format(time / 1000 + i, satellite, pos_x, pos_y,
                                                                           pos_z, velocity))
        x0 = x0 + 1


# Satellites and ground stations start positions generation
def generate_start_positions(timestamp, positions):
    names_file = open("./data/data.trace", "w+")
    satellites = 40
    ground_stations = 9
    satellite = 0

    # Satellites
    for satellite in range(satellites):
        names_file.write('$node_({}) set X_ {}\n'.format(satellite, positions[satellite][0][0]))
        names_file.write('$node_({}) set Y_ {}\n'.format(satellite, positions[satellite][0][1]))
        names_file.write('$node_({}) set Z_ {}\n'.format(satellite, positions[satellite][0][2]))

    # Gateways
    for ground_station in range(satellites, satellites + ground_stations):
        names_file.write('$node_({}) set X_ {}\n'.format(ground_station, positions[ground_station][0][0]))
        names_file.write('$node_({}) set Y_ {}\n'.format(ground_station, positions[ground_station][0][1]))
        names_file.write('$node_({}) set Z_ {}\n'.format(ground_station, positions[ground_station][0][2]))
    # Ground stations
    for ground_station in range(satellites, satellites + ground_stations):
        names_file.write('$node_({}) set X_ {}\n'.format(ground_station + ground_stations, positions[ground_station][0][0]))
        names_file.write('$node_({}) set Y_ {}\n'.format(ground_station + ground_stations, positions[ground_station][0][1]))
        names_file.write('$node_({}) set Z_ {}\n'.format(ground_station + ground_stations, positions[ground_station][0][2]))
    names_file.close()


def generate_trace(timestamp, positions, velocities):
    # Generate start positions:
    generate_start_positions(timestamp, positions)
    # Trace generation:
    names_file = open("./data/data.trace", "a")
    satellites = 40
    ground_stations = 9
    x = 0
    final_index = 100
    if final_index > len(timestamp):
        final_index = len(timestamp)
    for time in range(final_index):
        for satellite in range(satellites):
            pos_x = positions[satellite][x][0]
            pos_y = positions[satellite][x][1]
            pos_z = positions[satellite][x][2]
            vel_x = velocities[satellite][x][0]
            vel_y = velocities[satellite][x][1]
            vel_z = velocities[satellite][x][2]
            velocity = get_velocity(vel_x, vel_y, vel_z)
            names_file.write('$ns_ at {} "$node_({}) setdest {} {} {} {}"\n'.format(timestamp[time] / 1000.0,
                                                                                    satellite,
                                                                                    pos_x,
                                                                                    pos_y,
                                                                                    pos_z,
                                                                                    velocity))
        # Gateways has the same position and velocity
        for ground_station in range(satellites, satellites + ground_stations):
            pos_x = positions[ground_station][x][0]
            pos_y = positions[ground_station][x][1]
            pos_z = positions[ground_station][x][2]
            vel_x = velocities[ground_station][x][0]
            vel_y = velocities[ground_station][x][1]
            vel_z = velocities[ground_station][x][2]
            velocity = get_velocity(vel_x, vel_y, vel_z)
            names_file.write('$ns_ at {} "$node_({}) setdest {} {} {} {}"\n'.format(timestamp[time] / 1000.0,
                                                                                    ground_station,
                                                                                    pos_x,
                                                                                    pos_y,
                                                                                    pos_z,
                                                                                    velocity))
        # Ground stations:
        for ground_station in range(satellites, satellites + ground_stations):
            pos_x = positions[ground_station][x][0]
            pos_y = positions[ground_station][x][1]
            pos_z = positions[ground_station][x][2]
            vel_x = velocities[ground_station][x][0]
            vel_y = velocities[ground_station][x][1]
            vel_z = velocities[ground_station][x][2]
            velocity = get_velocity(vel_x, vel_y, vel_z)
            names_file.write('$ns_ at {} "$node_({}) setdest {} {} {} {}"\n'.format(timestamp[time] / 1000.0,
                                                                                    ground_station + ground_stations,
                                                                                    pos_x,
                                                                                    pos_y,
                                                                                    pos_z,
                                                                                    velocity))
        x = x + 1
    names_file.close()


def generate_links(timestamp, links):
    satellites = 40
    ground_stations = 9
    print("Links file creation...")
    file = open("./data/links", "w+")
    matrix_size = satellites + ground_stations * 2
    connected_components = []
    for moment_of_time in range(len(timestamp[:100])):
        file.write('{}s\n'.format(int(timestamp[moment_of_time] / 1000)))
        matrix = np.identity((matrix_size), dtype='int32')
        matrix[0:links.shape[1], 0:links.shape[2]] = links[moment_of_time]
        for i in range(ground_stations):
            matrix[i + satellites][i + satellites + ground_stations] = 1
        for i in range(ground_stations):
            matrix[i + satellites + ground_stations][i + satellites] = 1
        for row in matrix:
            for col in row:
                file.write('{} '.format(int(col)))
            file.write('\n')
        laplacian = np.diag(np.sum(matrix, axis=0)) - matrix
        eigen_vals, _ = np.linalg.eig(laplacian)
        connected_components += [np.sum(eigen_vals <= np.finfo(np.float32).eps)]
    plt.hist(connected_components)
    plt.savefig("linked_components_gw.png")
    file.close()


def load_data(nodes_path, links_path):
    nodes = pd.read_csv(nodes_path, sep=';')
    links = pd.read_csv(links_path, sep=';')
    assert check_data(nodes, links)
    header = list(nodes.columns.values)
    n_stations = int(len(header) - 1) / (2 * DIM)
    print('Expected {0} nodes.'.format(n_stations))

    timestamp = nodes['Time(ms)'].as_matrix()
    nodes = nodes.as_matrix()[:, 1:]  # Strip timestamps
    links = links.as_matrix()[:, 1:]  # Strip timestamps

    positions = {}
    velocities = {}
    for i in range(int(n_stations)):
        positions[i] = nodes[:, 2 * DIM * i:2 * DIM * i + DIM]
        velocities[i] = nodes[:, 2 * DIM * i + DIM:2 * DIM * (i + 1)]

    generate_trace(timestamp, positions, velocities)
    generate_links(timestamp, links.reshape(-1, int(n_stations), int(n_stations)))

    return {
        'timestamp': timestamp,
        'positions': positions,
        'velocities': velocities,
        'links': links.reshape(-1, int(n_stations), int(n_stations))
    }

def plot_scene(network_data, ax, time_index):
    # draw Earth sphere
    u, v = np.mgrid[0:2 * np.pi:20j, 0:np.pi:10j]
    phi = network_data['timestamp'][time_index] / 1e3 * EARTH_ROTATION
    x = EARTH_RADIUS * np.cos(u + phi) * np.sin(v)
    y = EARTH_RADIUS * np.sin(u + phi) * np.sin(v)
    z = EARTH_RADIUS * np.cos(v)
    ax.plot_wireframe(x, y, z, color="g")

    # Plot vertices
    point_size = 100
    for sta_pos in network_data['positions'].values():
        pos_vec = sta_pos[time_index, :]
        color = 'b' if np.linalg.norm(pos_vec) > EARTH_RADIUS + MIN_SAT_ALT else 'r'
        ax.scatter(*pos_vec, color=color, s=point_size)
    # plot links
    n_stations = len(network_data['positions'].keys())
    for i in range(n_stations):
        for j in range(i, n_stations):

            if network_data['links'][time_index, i, j]:
                src_pos = network_data['positions'][i][time_index, :]
                dst_pos = network_data['positions'][j][time_index, :]
                ax.plot(
                    [src_pos[0], dst_pos[0]],
                    [src_pos[1], dst_pos[1]],
                    [src_pos[2], dst_pos[2]],
                    color='b'
                )


def draw_satellites(connected_components):
    from mpl_toolkits.mplot3d import axes3d, Axes3D
    for i in range(14000):
        limits = EARTH_RADIUS + 1e6
        fig = plt.figure(figsize=(20,20))
        ax = Axes3D(fig)
        ax.set_xlim3d(-limits, limits)
        ax.set_ylim3d(-limits, limits)
        ax.set_zlim3d(-limits, limits)
        ax.set_aspect("equal")
        plot_scene(connected_components, ax, i)
        plt.savefig('data/frames/frame_%05d.png' % i, bbox_inches='tight', dpi=100)
        plt.clf()
        plt.close(fig)


if __name__ == '__main__':
    import os
    import matplotlib.pyplot as plt

    # Data prvoded by the propagator. Note, that header is removed for easier processing.
    # The header describes the set of satellites and ground stations. Results are provided in EME2000 reference frame
    # Note, that all spaces and headers are removed before loading the data
    data_root = 'data'
    data = load_data(os.path.join(data_root, 'nodes.csv'), os.path.join(data_root, 'links.csv'))