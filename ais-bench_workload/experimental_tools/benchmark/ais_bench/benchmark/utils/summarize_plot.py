import plotly.graph_objects as go
from plotly.subplots import make_subplots
from mmengine.logging import MMLogger
import numpy as np

def plot_sorted_request_timelines(request_times, output_file: str = "timeline.html", unit : str = "ms", logger: MMLogger = None):
    
    if logger:
        logger.info("Rendering request timeline diagram...")

    # ================== WebGL 配置 ==================
    WEBGL_CONFIG = {
        'scrollZoom': True,
        'plotGlPixelRatio': 0.8,  # 渲染分辨率
        'showLink': False,
        'displaylogo': False,
        'modeBarButtonsToRemove': ['toImage'],
        'queueLength': 10,        # 渲染队列长度
    }
    
    rescale = {
        "s": 1000,
        "ms": 1,
    }
    rescale_factor = rescale[unit]

    # ================== 向量化数据处理 ==================
    if logger:
        logger.info("Processing the received data...")

    request_times_np = np.array(
        [[float(t) for t in req] for req in request_times],
        dtype=object
    )
    
    # 计算全局最小时间
    all_times = np.concatenate(request_times_np)
    global_x_min = np.min(all_times) if all_times.size > 0 else 0
    
    # 调整所有时间值为相对值
    adjusted_requests = [
        (np.array(req) / rescale_factor) - (global_x_min / rescale_factor)
        for req in request_times
    ]

    if logger:
        logger.info("Data preprocessing completed! Processing data for request timeline diagram...")

    # ================== 请求图：请求线段图处理及渲染 ==================
    # 获取有效请求索引（至少包含2个时间点）
    valid_indices = [i for i, req in enumerate(adjusted_requests) if len(req) >= 2]
    n_valid = len(valid_indices)
    
    if n_valid == 0:
        if logger:
            logger.warning("Data preprocessing completed! Processing data for request timeline diagram...")
        return

    # ================== 预分配内存 ==================
    # 红线段数据（每个有效请求3个点：起点、终点、None）
    red_x = np.full(3 * n_valid, np.nan, dtype=np.float32)
    red_y = np.full(3 * n_valid, np.nan, dtype=np.float32)
    
    # 蓝线段数据（同样结构）
    blue_x = np.full(3 * n_valid, np.nan, dtype=np.float32)
    blue_y = np.full(3 * n_valid, np.nan, dtype=np.float32)
    
    # 悬停文本（仅在起点存储）
    hover_text = np.full(3 * n_valid, None, dtype=object)
    
    # 提取第一个时间点用于排序
    first_times = np.array([req[0] for req in adjusted_requests if len(req) >= 2])
    sorted_indices = np.argsort(first_times)
    
    # ================== 填充线段数据 ==================
    for sorted_pos, orig_idx in enumerate(sorted_indices):
        req_idx = valid_indices[orig_idx]
        times = adjusted_requests[req_idx]
        
        # 计算数组中的位置
        arr_idx = sorted_pos * 3
        
        # 红线段（TTFT）
        red_x[arr_idx] = times[0]
        red_x[arr_idx + 1] = times[1]
        red_y[arr_idx:arr_idx + 2] = sorted_pos + 1
        
        # 蓝线段（解码时间）
        if times[-1] > times[1]:
            blue_x[arr_idx] = times[1]
            blue_x[arr_idx + 1] = times[-1]
            blue_y[arr_idx:arr_idx + 2] = sorted_pos + 1
        
        # 悬停文本（仅在起点设置）
        ttft = times[1] - times[0]
        decode_time = times[-1] - times[1]
        e2e = times[-1] - times[0]
        
        hover_text[arr_idx] = (
            f"<span style='color:red'>TTFT({unit}): {times[0]:.2f}→{times[1]:.2f}={ttft:.2f}</span><br>"
            f"<span style='color:blue'>Decode({unit}): {times[1]:.2f}→{times[-1]:.2f}={decode_time:.2f}</span><br>"
            f"E2EL({unit}): {times[0]:.2f}→{times[-1]:.2f}={e2e:.2f}"
        )
    
    if logger:
        logger.info("Request timeline diagram data processing complete! Rendering...")
    
    # ================== 请求图：分块渲染 ==================
    CHUNK_SIZE = 5000  # 每个轨迹包含5000个请求
    n_chunks = (n_valid + CHUNK_SIZE - 1) // CHUNK_SIZE
    traces_row1 = []

    # ================== 创建分块轨迹 ==================
    for i in range(n_chunks):
        start_idx = i * CHUNK_SIZE * 3
        end_idx = min((i + 1) * CHUNK_SIZE * 3, len(red_x))
        chunk = slice(start_idx, end_idx)
        
        # 红色轨迹（带悬停）
        traces_row1.append(go.Scattergl(
            x=red_x[chunk],
            y=red_y[chunk],
            mode='lines',
            line=dict(color='red', width=1, shape="hv"),
            hoverinfo='text',
            hovertext=hover_text[chunk],
            showlegend=False,
            connectgaps=False
        ))
        
        # 蓝色轨迹（无悬停）
        traces_row1.append(go.Scattergl(
            x=blue_x[chunk],
            y=blue_y[chunk],
            mode='lines',
            line=dict(color='blue', width=1, shape="hv"),
            hoverinfo='none',
            showlegend=False,
            connectgaps=False
        ))

    if logger:
        logger.info("Request timeline diagram rendering complete! Processing data for request concurrency line chart...")

    # ================== 并发图：请求并发数量线段图处理及渲染 ==================
    # 向量化事件生成
    starts = np.array([req[0] for req in adjusted_requests if len(req) >= 1], dtype=np.float32)
    ends = np.array([req[-1] for req in adjusted_requests if len(req) >= 1], dtype=np.float32)
    
    # 创建事件数组并排序
    events = np.empty((len(starts) + len(ends), 2), dtype=np.float32)
    events[:len(starts), 0] = starts
    events[:len(starts), 1] = 1
    events[len(starts):, 0] = ends
    events[len(starts):, 1] = -1

    events = events[events[:, 0].argsort()]
    
    # 计算并发数（高效向量化), 合并相同时间点的事件
    unique_times, inverse_indices = np.unique(events[:, 0], return_inverse=True)
    time_deltas = np.bincount(inverse_indices, weights=events[:, 1])
    cumulative = np.cumsum(time_deltas)
    
    # 3. 生成阶梯图数据
    step_x = np.empty(2 * len(unique_times), dtype=np.float32)
    step_y = np.empty(2 * len(unique_times), dtype=np.float32)
    
    # 起始点
    step_x[0] = unique_times[0]
    step_y[0] = 0
    
    # 向量化填充阶梯数据
    for i in range(1, len(unique_times)):
        step_x[2*i-1] = unique_times[i]
        step_y[2*i-1] = cumulative[i-1]
        step_x[2*i] = unique_times[i]
        step_y[2*i] = cumulative[i-1]
    
    # 添加最后一个点
    step_x[-1] = unique_times[-1]
    step_y[-1] = cumulative[-1]
    
    if logger:
        logger.info("Request concurrency line chart data processing complete! Rendering...")

    # ================== 并发图：分块渲染 ==================
    CONCURRENCY_CHUNK_SIZE = 20000  # 每个并发轨迹包含2万个点
    n_points = len(step_x)
    n_concurrency_chunks = (n_points + CONCURRENCY_CHUNK_SIZE - 1) // CONCURRENCY_CHUNK_SIZE
    traces_row2 = []

    for i in range(n_concurrency_chunks):
        start_idx = i * CONCURRENCY_CHUNK_SIZE
        end_idx = min((i + 1) * CONCURRENCY_CHUNK_SIZE, n_points)
        
        # 确保连续：每块的起始点包含上一块的结束点（第一块除外）
        if i > 0:
            start_idx = max(0, start_idx - 1)
        
        chunk_x = step_x[start_idx:end_idx]
        chunk_y = step_y[start_idx:end_idx]
        
        if len(chunk_x) == 0:
            continue
            
        # 创建并发轨迹块
        trace = go.Scattergl(
            x=chunk_x,
            y=chunk_y,
            mode='lines',
            line=dict(color='#4CAF50', width=1, shape='hv'),
            fill='tozeroy',
            fillcolor='rgba(76,175,80,0.1)',
            hovertemplate="Timestamp: %{x:.2f}<br>Concurrency: %{y:.0f}<extra></extra>",
            showlegend=False,
            connectgaps=True
        )
        traces_row2.append(trace)
    
    if logger:
        logger.info("Request concurrency line chart rendering complete! Merging both diagrams...")

    # ================== 合并图表 ==================
    combined_fig = make_subplots(rows=2, cols=1, vertical_spacing=0.1)
    
    # 添加请求图轨迹
    for trace in traces_row1:
        combined_fig.add_trace(trace, row=1, col=1)
    
    # 添加并发图轨迹
    for trace in traces_row2:
        combined_fig.add_trace(trace, row=2, col=1)

    # ================== 坐标轴配置 ==================
    axis_config = dict(
        showline=True,
        showgrid=True,
        gridwidth=0.5,
        gridcolor='rgba(211,211,211,0.5)',
        linecolor='black',
    )
    
    # 计算X轴范围
    all_times = np.concatenate([red_x[~np.isnan(red_x)], blue_x[~np.isnan(blue_x)]])
    x_range = [0, np.max(all_times)] if all_times.size > 0 else [0, 1]
    
    xaxis_config = dict(
        showspikes=True,
        spikemode='across',
        spikesnap='cursor',
        spikethickness=1,
        spikecolor='#666',
        spikedash='dot',
        title=f"Relative Time ({unit})",
        range=x_range,
    )
    
    yaxis_config = dict(
        rangemode='nonnegative',
        tickmode='auto',
        nticks=10,
    )
    
    combined_fig.update_layout(
        height=1200,
        plot_bgcolor='white',
        xaxis1=dict(
            **axis_config,
            **xaxis_config,
            matches='x2',
        ),
        yaxis1=dict(
            **axis_config,
            **yaxis_config,
            title="Request Index", 
        ),
        xaxis2=dict(
            **axis_config,
            **xaxis_config,
        ),
        yaxis2=dict(
            **axis_config,
            **yaxis_config,
            title="Request Concurrency Count",
        ),
        hoverlabel=dict(
            bgcolor='rgba(255,255,255,0.9)',
            font_size=12,
            align='left'
        ),
        hovermode='closest',
    )

    # ================== 输出HTML ==================
    combined_fig.write_html(
        output_file,
        include_plotlyjs='cdn',
        config=WEBGL_CONFIG,
        auto_open=False,
        include_mathjax='cdn',
        full_html=False
    )

    if logger:
        logger.info("Merged diagram rendering complete! HTML file generated!")

    # 清理大数组释放内存
    del red_x, red_y, blue_x, blue_y, hover_text